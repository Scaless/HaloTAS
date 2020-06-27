#include "tas_hooks.h"
#include "windows_utilities.h"
#include "patch.h"
#include "hook.h"
#include "tas_overlay.h"
#include "kiero.h"

#include <winternl.h>
#include <string>
#include <mutex>
#include <unordered_map>

static std::once_flag gOverlayInitialized;
// Global hooks should only apply to areas that will never be unloaded
static std::vector<hook> gGlobalHooks;
// Runtime hooks & patches are applied and unapplied when dlls are loaded and unloaded
static std::vector<hook> gRuntimeHooks;
static std::vector<patch> gRuntimePatches;

// TODO-SCALES: Refactor this stuff out into respective classes
void attach_global_hooks();
void detach_global_hooks();
///////////////////////////////////////////////////////////////

typedef HMODULE(*LoadLibraryA_t)(LPCSTR lpLibFileName);
HMODULE hkLoadLibraryA(LPCSTR lpLibFileName);
LoadLibraryA_t originalLoadLibraryA;

typedef HMODULE(*LoadLibraryW_t)(LPCWSTR lpLibFileName);
HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName);
LoadLibraryW_t originalLoadLibraryW;

typedef HMODULE(*LoadLibraryExA_t)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
LoadLibraryExA_t originalLoadLibraryExA;

typedef HMODULE(*LoadLibraryExW_t)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
LoadLibraryExW_t originalLoadLibraryExW;

typedef BOOL(*FreeLibrary_t)(HMODULE hLibModule);
BOOL hkFreeLibrary(HMODULE hLibModule);
FreeLibrary_t originalFreeLibrary;

typedef uint8_t(*MCCGetHalo1Input_t)(int64_t, int64_t, char*);
uint8_t hkMCCGetHalo1Input(int64_t functionAddr, int64_t unknown, char* outValue);
MCCGetHalo1Input_t originalMCCHalo1Input;

typedef char (*Halo1HandleInput_t)();
char hkHalo1HandleInput();
Halo1HandleInput_t originalHalo1HandleInput;

typedef HRESULT(*D3D11Present_t)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
HRESULT hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
D3D11Present_t originalD3D11Present;

tas_hooks::tas_hooks()
{
	gRuntimePatches.emplace_back(L"halo1.dll", 0x077FD2F, std::vector<uint8_t>{ 0xB0, 0x01 });
}
tas_hooks::~tas_hooks()
{
	tas::overlay::shutdown();
}

void attach_global_hooks() {
	// D3D11 Hooking
	if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
		// See https://github.com/Rebzzel/kiero/blob/master/METHODSTABLE.txt for available functions
		auto methods = kiero::getMethodsTable();

		originalD3D11Present = (D3D11Present_t)methods[8];
		gGlobalHooks.emplace_back(&(PVOID&)originalD3D11Present, hkD3D11Present);
	}

	// Windows API Hooking
	originalLoadLibraryA = LoadLibraryA;
	gGlobalHooks.emplace_back(&(PVOID&)originalLoadLibraryA, hkLoadLibraryA);
	originalLoadLibraryW = LoadLibraryW;
	gGlobalHooks.emplace_back(&(PVOID&)originalLoadLibraryW, hkLoadLibraryW);
	originalLoadLibraryExA = LoadLibraryExA;
	gGlobalHooks.emplace_back(&(PVOID&)originalLoadLibraryExA, hkLoadLibraryExA);
	originalLoadLibraryExW = LoadLibraryExW;
	gGlobalHooks.emplace_back(&(PVOID&)originalLoadLibraryExW, hkLoadLibraryExW);
	originalFreeLibrary = FreeLibrary;
	gGlobalHooks.emplace_back(&(PVOID&)originalFreeLibrary, hkFreeLibrary);

	HMODULE mccModule = GetModuleHandle(L"MCC-Win64-Shipping.exe");
	originalMCCHalo1Input = (MCCGetHalo1Input_t)((uint8_t*)mccModule + 0x18C5A44);
	gGlobalHooks.emplace_back(&(PVOID&)originalMCCHalo1Input, hkMCCGetHalo1Input);

	HMODULE h1Module = GetModuleHandle(L"halo1.dll");
	originalHalo1HandleInput = (Halo1HandleInput_t)((uint8_t*)h1Module + 0x70E430);
	gGlobalHooks.emplace_back(&(PVOID&)originalHalo1HandleInput, hkHalo1HandleInput);

	// Attach the hooks
	for (auto& hk : gGlobalHooks) {
		hk.attach();
	}
}

void detach_global_hooks() {
	for (auto& hk : gGlobalHooks) {
		hk.detach();
	}
}

void tas_hooks::attach_all() {
	attach_global_hooks();
	for (auto& hook : gRuntimeHooks) {
		hook.attach();
	}
	for (auto& patch : gRuntimePatches) {
		patch.apply();
	}
}

void tas_hooks::detach_all() {
	for (auto& hook : gRuntimeHooks) {
		hook.detach();
	}
	for (auto& patch : gRuntimePatches) {
		patch.restore();
	}
	detach_global_hooks();
}

void post_lib_load_install_hooks(std::wstring_view libName) {

	for (auto& hook : gRuntimeHooks) {
		if (hook.module_name() == libName) {
			hook.attach();
			tas_logger::info(L"Installed Hook For {}", libName);
		}
	}
	for (auto& patch : gRuntimePatches) {
		if (patch.module_name() == libName) {
			patch.apply();
			tas_logger::info(L"Installed Patch For {}", libName);
		}
	}
}

HMODULE hkLoadLibraryA(LPCSTR lpLibFileName) {
	auto result = originalLoadLibraryA(lpLibFileName);

	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, lpLibFileName, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, lpLibFileName, -1, wstr, wchars_num);

	tas_logger::trace(L"LoadLibraryA: {}", wstr);

	return result;
}

HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName) {
	auto result = originalLoadLibraryW(lpLibFileName);

	tas_logger::trace(L"LoadLibraryW: {}", lpLibFileName);

	return result;
}

HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = originalLoadLibraryExA(lpLibFileName, hFile, dwFlags);

	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, lpLibFileName, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, lpLibFileName, -1, wstr, wchars_num);

	tas_logger::trace(L"LoadLibraryExA: {}", wstr);

	return result;
}

HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = originalLoadLibraryExW(lpLibFileName, hFile, dwFlags);

	tas_logger::trace(L"LoadLibraryExW: {}", lpLibFileName);

	post_lib_load_install_hooks(lpLibFileName);

	return result;
}

BOOL hkFreeLibrary(HMODULE hLibModule) {
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(hLibModule, buffer, sizeof(buffer) / sizeof(TCHAR));
	tas_logger::trace(L"FreeLibrary: {}", buffer);

	// Detach hooks before unload
	for (auto& hook : gRuntimeHooks) {
		if (hook.module_name() == buffer) {
			hook.detach();
		}
	}
	for (auto& patch : gRuntimePatches) {
		if (patch.module_name() == buffer) {
			patch.restore();
		}
	}

	return originalFreeLibrary(hLibModule);
}

HRESULT hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {
	std::call_once(gOverlayInitialized, tas::overlay::initialize, SwapChain);

	tas::overlay::render();

	return originalD3D11Present(SwapChain, SyncInterval, Flags);
}

struct input {
	uint8_t everything[520];
};

char hkHalo1HandleInput() {
	static bool recording = false;
	static bool playback = false;
	static std::unordered_map<int, input> input_map;
	
	uint8_t* inputaddr = (uint8_t*)0x18218E2B0;
	int32_t* currenttime = (int32_t*)(0x18218E88C);
	int32_t* lasttabpress = (int32_t*)(0x18218E630);
	int8_t* tabdown = (int8_t*)(0x18218E2CE);
	int* absoluteMCCTick = (int*)(0x181176118);

	void** runtimeBase = (void**)0x18218E250;

	if (runtimeBase == nullptr) {
		return originalHalo1HandleInput();
	}

	int* tick = (int*)((char*)*runtimeBase + 0x2F4);

	char result = 0;
	if (!playback) {
		result = originalHalo1HandleInput();
	}

	if (*tick == 0 && recording && !playback) {
		tas_logger::info("Playback started");
		recording = false;
		playback = true;
	}
	if (*tick == 0 && !recording && !playback) {
		tas_logger::info("Recording started");
		recording = true;
	}

	if (recording) {
		if (input_map.find(*tick) == input_map.end()) {
			input i;
			memcpy(&i, inputaddr, sizeof(i));
			input_map[*tick] = i;
		}
		else {
			float* mouseX = (float*)&input_map[*tick].everything[500];
			float* mouseY = (float*)&input_map[*tick].everything[504];
			
			float* newMouseX = (float*)&inputaddr[500];
			float* newMouseY = (float*)&inputaddr[504];

			*mouseX += *newMouseX;
			*mouseY += *newMouseY;
		}
	}

	static int lasttick = *tick;

	if (playback) {
		if (input_map.find(*tick) != input_map.end()) {

			input i = input_map.at(*tick);
			float* mouseX = (float*)&inputaddr[500];
			*mouseX = *mouseX / 3.1f;
			float* mouseY = (float*)&inputaddr[504];
			*mouseY = *mouseY / 3.1f;
			memcpy(inputaddr, &i, sizeof(i));
			
			if (lasttick == *tick) {
				*mouseX = 0;
				*mouseY = 0;
			}

			lasttick = *tick;
		}
	}

	//if (*tick != lasttick) {
	//	*tabdown = 1;
	//	lasttick = *tick;
	//}
	//else {
	//	*tabdown = 0;
	//}

	//*currenttime += 1000;

	return result;
}

uint8_t hkMCCGetHalo1Input(int64_t functionAddr, int64_t unknown, char* vkeyTable) {

	// Pre-Input
	auto MCCReturn = originalMCCHalo1Input(functionAddr, unknown, vkeyTable);
	// Post-Input

	// This is where we set our own inputs

	//uint8_t val = vkeyTable[0];
	//tas_logger::info("{}", val);

	//int32_t* tick = (int32_t*)(0x1957B0D02F4);
	//static int32_t lasttick = *tick;

	//if (lasttick != *tick) {
	//	static bool flipflop = false;
	//	vkeyTable[4 + VK_TAB] = 1;
	//	//flipflop = !flipflop;
	//	//vkeyTable[4 + VK_TAB] = flipflop ? 1 : 0;
	//	lasttick = *tick;
	//}
	//else {
	//	vkeyTable[4 + VK_TAB] = 0;
	//}

	//float* mouseX = (float*)(vkeyTable + 260);
	//float* mouseY = (float*)(vkeyTable + 264);

	return MCCReturn;
}

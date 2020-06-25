#include "tas_hooks.h"
#include "windows_utilities.h"
#include "patch.h"
#include "hook.h"
#include "tas_overlay.h"
#include "kiero.h"

#include <winternl.h>
#include <string>
#include <mutex>

static std::once_flag gOverlayInitialized;
static std::vector<hook> gGlobalHooks;
static std::vector<hook> gGameHooks;
static std::vector<patch> gGamePatches;

// TODO-SCALES: Refactor this stuff out into respective classes
void attach_game_hooks();
void detach_game_hooks();
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

typedef HRESULT(*D3D11Present_t)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
HRESULT hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
D3D11Present_t originalD3D11Present;

tas_hooks::tas_hooks()
{
	gGamePatches.emplace_back(L"halo1.dll", 0x077FD2F, std::vector<uint8_t>{ 0xB0, 0x01 });
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

void attach_game_hooks() {
	for (auto& hook : gGameHooks) {
		hook.attach();
	}
	for (auto& patch : gGamePatches) {
		patch.apply();
	}
}

void detach_game_hooks() {
	for (auto& hook : gGameHooks) {
		hook.detach();
	}
	for (auto& patch : gGamePatches) {
		patch.restore();
	}
}

void tas_hooks::attach_all() {
	attach_global_hooks();
	attach_game_hooks();
}

void tas_hooks::detach_all() {
	detach_game_hooks();
	detach_global_hooks();
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

	std::wstring libName(lpLibFileName);
	tas_logger::trace(L"LoadLibraryExW: {}", lpLibFileName);

	// TODO-SCALES: apply to all loadlibrary calls
	if (libName == L"halo1.dll") {
		tas_logger::trace(L"Installing Hooks & Patches For {}", libName.c_str());
		attach_game_hooks();
	}

	return result;
}

BOOL hkFreeLibrary(HMODULE hLibModule) {
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(hLibModule, buffer, sizeof(buffer) / sizeof(TCHAR));
	tas_logger::trace(L"FreeLibrary: {}", buffer);

	return originalFreeLibrary(hLibModule);
}

HRESULT hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {
	std::call_once(gOverlayInitialized, tas::overlay::initialize, SwapChain);

	tas::overlay::render();

	return originalD3D11Present(SwapChain, SyncInterval, Flags);
}

uint8_t hkMCCGetHalo1Input(int64_t functionAddr, int64_t unknown, char* vkeyTable) {

	// Pre-Input
	auto MCCReturn = originalMCCHalo1Input(functionAddr, unknown, vkeyTable);
	// Post-Input

	// This is where we set our own inputs

	//static bool flipflop = false;
	//flipflop = !flipflop;
	//vkeyTable[4 + VK_TAB] = flipflop ? 1 : 0;

	return MCCReturn;
}

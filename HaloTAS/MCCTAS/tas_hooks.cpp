#include "tas_hooks.h"
#include "windows_utilities.h"
#include "patch.h"
#include "hook.h"
#include "tas_overlay.h"
#include "tas_input.h"
#include "kiero.h"
#include "dll_cache.h"
#include "halo1_engine.h"

#include <winternl.h>
#include <string>
#include <mutex>
#include <unordered_map>
#include <filesystem>
#include <chrono>

static std::once_flag gOverlayInitialized;
// Global hooks should only apply to areas that will never be unloaded
static std::vector<hook> gGlobalHooks;
// Runtime hooks & patches are applied and unapplied when dlls are loaded and unloaded
static std::vector<hook> gRuntimeHooks;
static std::vector<patch> gRuntimePatches;

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

typedef uint8_t(*MCCGetInput_t)(int64_t, int64_t, MCCInput*);
uint8_t hkMCCGetInput(int64_t functionAddr, int64_t unknown, MCCInput* inputTable);
MCCGetInput_t originalMCCInput;

typedef int64_t(*H1GetNumberOfTicksToTick)(float a1, uint8_t a2);
int64_t hkH1GetNumberOfTicksToTick(float a1, uint8_t a2);
H1GetNumberOfTicksToTick originalH1GetNumberOfTicksToTick;

typedef char (*Halo1HandleInput_t)();
char hkHalo1HandleInput();
Halo1HandleInput_t originalHalo1HandleInput;

typedef HRESULT(*D3D11Present_t)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
HRESULT hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
D3D11Present_t originalD3D11Present;

const patch RuntimePatch_EnableH1DevConsole(L"EnableH1DevConsole", HALO1_DLL_WSTR, halo1::patch::OFFSET_ENABLE_DEV_CONSOLE, halo1::patch::PATCHBYTES_ENABLE_DEV_CONSOLE);

const hook RuntimeHook_Halo1HandleInput(L"hkHalo1HandleInput", HALO1_DLL_WSTR, halo1::function::OFFSET_H1_HANDLE_INPUT, (PVOID**)&originalHalo1HandleInput, hkHalo1HandleInput);
const hook RuntimeHook_Halo1GetNumberOfTicksToTick(L"hkH1GetNumberOfTicksToTick", HALO1_DLL_WSTR, halo1::function::OFFSET_H1_GET_NUMBER_OF_TICKS, (PVOID**)&originalH1GetNumberOfTicksToTick, hkH1GetNumberOfTicksToTick);

const hook GlobalHook_D3D11Present(L"hkD3D11Present", (PVOID**)&originalD3D11Present, hkD3D11Present);
const hook GlobalHook_LoadLibraryA(L"hkLoadLibraryA", (PVOID**)&originalLoadLibraryA, hkLoadLibraryA);
const hook GlobalHook_LoadLibraryW(L"hkLoadLibraryW", (PVOID**)&originalLoadLibraryW, hkLoadLibraryW);
const hook GlobalHook_LoadLibraryExA(L"hkLoadLibraryExA", (PVOID**)&originalLoadLibraryExA, hkLoadLibraryExA);
const hook GlobalHook_LoadLibraryExW(L"hkLoadLibraryExW", (PVOID**)&originalLoadLibraryExW, hkLoadLibraryExW);
const hook GlobalHook_FreeLibrary(L"hkFreeLibrary", (PVOID**)&originalFreeLibrary, hkFreeLibrary);
const hook GlobalHook_MCCGetHalo1Input(L"hkMCCGetInput", L"MCC-Win64-Shipping.exe", mcc::function::OFFSET_MCCGETINPUT, (PVOID**)&originalMCCInput, hkMCCGetInput);

tas_hooks::tas_hooks()
{
	gRuntimePatches.push_back(RuntimePatch_EnableH1DevConsole);
	gRuntimeHooks.push_back(RuntimeHook_Halo1HandleInput);
	gRuntimeHooks.push_back(RuntimeHook_Halo1GetNumberOfTicksToTick);

	gGlobalHooks.push_back(GlobalHook_D3D11Present);
	gGlobalHooks.push_back(GlobalHook_LoadLibraryA);
	gGlobalHooks.push_back(GlobalHook_LoadLibraryW);
	gGlobalHooks.push_back(GlobalHook_LoadLibraryExA);
	gGlobalHooks.push_back(GlobalHook_LoadLibraryExW);
	gGlobalHooks.push_back(GlobalHook_FreeLibrary);
	gGlobalHooks.push_back(GlobalHook_MCCGetHalo1Input);

	init_global_hooks();
}
tas_hooks::~tas_hooks()
{
	tas::overlay::shutdown();
}

void tas_hooks::attach_global_hooks() {
	for (auto& hk : gGlobalHooks) {
		hk.attach();
	}
}

void tas_hooks::detach_global_hooks() {
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

void tas_hooks::init_global_hooks()
{
	// D3D11 Hooking
	if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
		// See https://github.com/Rebzzel/kiero/blob/master/METHODSTABLE.txt for available functions
		auto methods = kiero::getMethodsTable();
		originalD3D11Present = (D3D11Present_t)methods[8];
	}

	// Windows API Hooking
	originalLoadLibraryA = LoadLibraryA;
	originalLoadLibraryW = LoadLibraryW;
	originalLoadLibraryExA = LoadLibraryExA;
	originalLoadLibraryExW = LoadLibraryExW;
	originalFreeLibrary = FreeLibrary;
}

void post_lib_load_hooks_patches(std::wstring_view libPath) {
	std::filesystem::path path = libPath;
	auto filename = path.filename().generic_wstring();

	for (auto& hook : gRuntimeHooks) {
		if (hook.module_name() == filename) {
			hook.attach();
		}
	}
	for (auto& patch : gRuntimePatches) {
		if (patch.module_name() == filename) {
			patch.apply();
		}
	}
}

void pre_lib_unload_hooks_patches(std::wstring_view libFilename) {
	for (auto& hook : gRuntimeHooks) {
		if (hook.module_name() == libFilename) {
			hook.detach();
		}
	}
	for (auto& patch : gRuntimePatches) {
		if (patch.module_name() == libFilename) {
			patch.restore();
		}
	}
}

HMODULE hkLoadLibraryA(LPCSTR lpLibFileName) {
	auto result = originalLoadLibraryA(lpLibFileName);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	tas_logger::trace(L"LoadLibraryA: {}", wLibFileName);
	dll_cache::add_to_cache(wLibFileName, result);
	post_lib_load_hooks_patches(wLibFileName);

	return result;
}

HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName) {
	auto result = originalLoadLibraryW(lpLibFileName);

	tas_logger::trace(L"LoadLibraryW: {}", lpLibFileName);
	dll_cache::add_to_cache(lpLibFileName, result);
	post_lib_load_hooks_patches(lpLibFileName);

	return result;
}

HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = originalLoadLibraryExA(lpLibFileName, hFile, dwFlags);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	tas_logger::trace(L"LoadLibraryExA: {}", wLibFileName);
	dll_cache::add_to_cache(wLibFileName, result);
	post_lib_load_hooks_patches(wLibFileName);

	return result;
}

HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = originalLoadLibraryExW(lpLibFileName, hFile, dwFlags);

	tas_logger::trace(L"LoadLibraryExW: {}", lpLibFileName);
	dll_cache::add_to_cache(lpLibFileName, result);
	post_lib_load_hooks_patches(lpLibFileName);

	return result;
}

BOOL hkFreeLibrary(HMODULE hLibModule) {
	wchar_t moduleFilePath[MAX_PATH];
	GetModuleFileName(hLibModule, moduleFilePath, sizeof(moduleFilePath) / sizeof(TCHAR));
	tas_logger::trace(L"FreeLibrary: {}", moduleFilePath);

	std::filesystem::path path = moduleFilePath;
	auto filename = path.filename().generic_wstring();

	pre_lib_unload_hooks_patches(filename);
	dll_cache::remove_from_cache(filename);

	return originalFreeLibrary(hLibModule);
}

tas_input InputCache;
tick_inputs CurrentTickInputs;

HRESULT hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {
	std::call_once(gOverlayInitialized, tas::overlay::initialize, SwapChain);

	tas::overlay::render(InputCache);

	return originalD3D11Present(SwapChain, SyncInterval, Flags);
}

struct input {
	uint8_t everything[520];
};

char hkHalo1HandleInput() {
	return originalHalo1HandleInput();
	/////////////////////////////////////////////

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

// The MCCGetInput function is used to pass input from the MCC parent process to each individual game DLL.
// Be careful when doing anything game-specific in here unless doing the proper checks!

bool globalStall = false;
bool recording = false;
bool playback = false;
bool forceTick = false;
int currentPlaybackFrame = 0;
uint8_t hkMCCGetInput(int64_t functionAddr, int64_t unknown, MCCInput* inputTable) {

	// Get input from MCC
	auto OriginalReturn = originalMCCInput(functionAddr, unknown, inputTable);

	// This is where we set our own inputs
	// Buffered: Space, CTRL, Tab , Mouse (& more) are buffered in the engine, multiple presses will be evaluated on the next tick.
	// Ex: Pressing tab twice will be processed on the next tick, resulting in no weapon swap because you would end up on the original weapon.
	// Other keys are immediate on the tick that they are needed, for example movement.

	if (GetAsyncKeyState(VK_ADD)) {
		recording = true;
		playback = false;
		tas_logger::warning("recording");
	}
	if (GetAsyncKeyState(VK_MULTIPLY)) {
		playback = true;
		recording = false;
		tas_logger::warning("playback");
	}
	if (GetAsyncKeyState(VK_SUBTRACT)) {
		playback = false;
		recording = false;
		tas_logger::warning("disabled");
	}

	if (GetAsyncKeyState(VK_NUMPAD0)) {
		globalStall = false;
		tas_logger::warning("stall stopped");
	}

	// Halo 1 code path
	auto H1DLL = dll_cache::get_info(HALO1_DLL_WSTR);
	if (H1DLL.has_value()) {
		int32_t** tick_base = (int32_t**)((uint8_t*)H1DLL.value() + halo1::data::OFFSET_TICK_BASE);
		if (*tick_base == nullptr) {
			return OriginalReturn;
		}

		if (globalStall) {
			return OriginalReturn;
		}

		int32_t* rng = (int32_t*)((uint8_t*)H1DLL.value() + halo1::data::OFFSET_RNG);
		int32_t tick = *(*tick_base + 3);
		static int32_t lasttick = tick;
		static int32_t absoluteTick = 0;
		static bool tickRevert = false;

		MCCInput* Input = (MCCInput*)inputTable;

		tas_logger::info("GetInput on tick {} | RNG({})", tick, *rng);

		if (tick == 0 && recording) {
			InputCache.reset();
			InputCache.start_tick(*rng);
		}
		
		if (tick == 0) {
			absoluteTick = 0;
			currentPlaybackFrame = 0;
		}

		bool ticked = false;
		if (lasttick != tick && tick != 0) {
			if (!(tick < lasttick && lasttick != 0)) { // Revert
				ticked = true;
			}
		}
		lasttick = tick;

		if (tick > 0 && recording && ticked) {
			InputCache.end_tick();
			InputCache.start_tick(*rng);
		}

		if (recording) {
			InputCache.push_input(*Input);
		}

		if (ticked) {
			// Goatrope speedometer
			float* cameraPositionArr = reinterpret_cast<float*>((uint8_t*)H1DLL.value() + 0x2199338);
			glm::vec3 currentCameraPosition(cameraPositionArr[0], cameraPositionArr[1], 0);
			static glm::vec3 previousCameraPosition = currentCameraPosition;

			float distance = glm::distance(previousCameraPosition, currentCameraPosition);
			previousCameraPosition = currentCameraPosition;
			tas::overlay::add_speed_value(distance);
			/////////////////////////
		}
		
		if (ticked) {
			absoluteTick++;
			currentPlaybackFrame = 0;
		}

		if (playback) {
			auto hasCurrentInput = InputCache.get_input(absoluteTick, currentPlaybackFrame, *rng);
			if (hasCurrentInput.has_value()) {
				auto currentInput = hasCurrentInput.value();
				memcpy_s(Input, sizeof(MCCInput), &currentInput.input, sizeof(currentInput.input));
				forceTick = currentInput.isLastFrame;
				currentPlaybackFrame++;

				if (currentInput.rngError) {
					globalStall = true;
				}
			}
			else {
				tas_logger::info("Tick({}) No input", absoluteTick);
				currentPlaybackFrame = 0;
				forceTick = true;
			}
		}

		// Just in case
		if (!playback) {
			forceTick = false;
		}

	}

	// DEFAULT
	return OriginalReturn;
}

int64_t hkH1GetNumberOfTicksToTick(float a1, uint8_t a2) {

	if (globalStall) {
		return 0;
	}

	if (playback) {
		if (forceTick) {
			return 1;
		}
		else {
			return 0;
		}
	}

	// The function is called multiple times in a frame loop
	// a2 is 0 when we are getting the actual amount of ticks to loop, otherwise it is usually 1
	if (a2) {
		return originalH1GetNumberOfTicksToTick(a1, a2);
	}
	
	auto originalReturn = originalH1GetNumberOfTicksToTick(a1, a2);

	bool Limit1TickPerFrame = true;
	if (Limit1TickPerFrame) {
		originalReturn = std::clamp<int64_t>(originalReturn, 0, 1);
	}

	if (GetAsyncKeyState(VK_RIGHT)) {
		originalReturn = 1;
	}

	bool superhot = false;
	if (superhot) {
		//                                        W                         A                         S                         D
		bool superhotKeyDown = GetAsyncKeyState(0x57) || GetAsyncKeyState(0x41) || GetAsyncKeyState(0x53) || GetAsyncKeyState(0x44)
			//                  SHIFT                         LEFT MOUSE                      RIGHT MOUSE
			|| GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RBUTTON);

		auto now = std::chrono::system_clock::now();
		static auto lastNow = now;

		int slowdownTicksPerSecond = 2;
		float slowdownMilliseconds = 1000.0f / slowdownTicksPerSecond;

		static auto slowTimer = std::chrono::system_clock::now();
		if (superhotKeyDown) {
			auto originalReturn = originalH1GetNumberOfTicksToTick(a1, a2);
			slowTimer = std::chrono::system_clock::now();
			lastNow = now;
			return originalReturn;
		}
		else {
			auto msdiff = std::chrono::duration_cast<std::chrono::milliseconds>(now - slowTimer);

			auto lastDiff = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastNow);
			lastNow = now;

			//lastDiff += std::chrono::milliseconds(1);

			tas_logger::info("{}", lastDiff.count());

			float deltaRealTime = lastDiff.count() / 1000.0f;
			float deltaTickTime = deltaRealTime * (slowdownTicksPerSecond / 30.0f);
			auto originalReturn = originalH1GetNumberOfTicksToTick(deltaTickTime, 0);
			if (msdiff.count() > slowdownMilliseconds) {
				if (originalReturn) {
					slowTimer = now;
				}
				return originalReturn;
			}
			return 0;
		}
	}

	return originalReturn;
}

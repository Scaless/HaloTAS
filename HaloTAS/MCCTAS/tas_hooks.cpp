#include "pch.h"
#include "tas_hooks.h"
#include "windows_utilities.h"
#include "patch.h"
#include "hook.h"
#include "tas_overlay.h"
#include "tas_input.h"
#include "dll_cache.h"
#include "halo1_engine.h"
#include "speedometer.h"
#include <d3d9.h>

static std::once_flag gOverlayInitialized;
// Global hooks should only apply to areas that will never be unloaded
static std::vector<hook> gGlobalHooks;
// Runtime hooks & patches are applied and unapplied when dlls are loaded and unloaded
static std::vector<hook> gRuntimeHooks;
static std::vector<patch> gRuntimePatches;

static std::shared_ptr<speedometer> speedo = std::make_shared<speedometer>(5, 30);

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

typedef int64_t(*H2Tick_t)(void* a1);
int64_t hkH2Tick(void* a1);
H2Tick_t originalH2Tick;

typedef void (*H2TickLoop_t)(int32_t a1, float* a2);
void hkH2TickLoop(int32_t a1, float* a2);
H2TickLoop_t originalH2TickLoop;

typedef void (*H3Tick_t)();
void hkH3Tick();
H3Tick_t originalH3Tick;

typedef HRESULT(*D3D11Present_t)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
HRESULT hkD3D11Present(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
D3D11Present_t originalD3D11Present;

typedef void (*D3D11RSSetState_t)(ID3D11Device* Ctx, ID3D11RasterizerState* pRasterizerState);
void hkD3D11RSSetState(ID3D11DeviceContext* Ctx, ID3D11RasterizerState* pRasterizerState);
D3D11RSSetState_t originalD3D11RSSetState;

typedef void (*D3D9SetRenderState_t)(IDirect3DDevice9* Device, D3DRENDERSTATETYPE State, DWORD Value);
void hkD3D9SetRenderState(IDirect3DDevice9* Device, D3DRENDERSTATETYPE State, DWORD Value);
D3D9SetRenderState_t originalD3D9SetRenderState;

typedef void (*D3D11Draw_t)(ID3D11DeviceContext* Ctx, UINT VertexCount, UINT StartVertexLocation);
void hkD3D11Draw(ID3D11DeviceContext* Ctx, UINT VertexCount, UINT StartVertexLocation);
D3D11Draw_t originalD3D11Draw;

typedef void (*D3D11DrawIndexed_t)(ID3D11DeviceContext* Ctx, UINT IndexCount,UINT StartIndexLocation,INT  BaseVertexLocation);
void hkD3D11DrawIndexed(ID3D11DeviceContext* Ctx, UINT IndexCount,UINT StartIndexLocation,INT  BaseVertexLocation);
D3D11DrawIndexed_t originalD3D11DrawIndexed;

const patch RuntimePatch_EnableH1DevConsole(L"EnableH1DevConsole", HALO1_DLL_WSTR, halo1::patch::OFFSET_ENABLE_DEV_CONSOLE, halo1::patch::PATCHBYTES_ENABLE_DEV_CONSOLE);

//const hook RuntimeHook_Halo1HandleInput(L"hkHalo1HandleInput", HALO1_DLL_WSTR, halo1::function::OFFSET_H1_HANDLE_INPUT, (PVOID**)&originalHalo1HandleInput, hkHalo1HandleInput);
const hook RuntimeHook_Halo1GetNumberOfTicksToTick(L"hkH1GetNumberOfTicksToTick", HALO1_DLL_WSTR, halo1::function::OFFSET_H1_GET_NUMBER_OF_TICKS, (PVOID**)&originalH1GetNumberOfTicksToTick, hkH1GetNumberOfTicksToTick);
//const hook RuntimeHook_Halo2Tick(L"hkHalo2Tick", HALO2_DLL_WSTR, halo2::function::OFFSET_H2_TICK, (PVOID**)&originalH2Tick, hkH2Tick);
//const hook RuntimeHook_Halo2TickLoop(L"hkHalo2TickLoop", HALO2_DLL_WSTR, halo2::function::OFFSET_H2_TICK_LOOP, (PVOID**)&originalH2TickLoop, hkH2TickLoop);
//const hook RuntimeHook_Halo3Tick(L"hkHalo3Tick", HALO3_DLL_WSTR, halo3::function::OFFSET_H3_TICK, (PVOID**)&originalH3Tick, hkH3Tick);

const hook GlobalHook_D3D9SetRenderState(L"hkD3D9SetRenderState", (PVOID**)&originalD3D9SetRenderState, hkD3D9SetRenderState);
const hook GlobalHook_D3D11Draw(L"hkD3D11Draw", (PVOID**)&originalD3D11Draw, hkD3D11Draw);
const hook GlobalHook_D3D11RSSetState(L"hkD3D11RSSetState", (PVOID**)&originalD3D11RSSetState, hkD3D11RSSetState);
const hook GlobalHook_D3D11Present(L"hkD3D11Present", (PVOID**)&originalD3D11Present, hkD3D11Present);
const hook GlobalHook_D3D11DrawIndexed(L"hkD3D11DrawIndexed", (PVOID**)&originalD3D11DrawIndexed, hkD3D11DrawIndexed);
const hook GlobalHook_LoadLibraryA(L"hkLoadLibraryA", (PVOID**)&originalLoadLibraryA, hkLoadLibraryA);
const hook GlobalHook_LoadLibraryW(L"hkLoadLibraryW", (PVOID**)&originalLoadLibraryW, hkLoadLibraryW);
const hook GlobalHook_LoadLibraryExA(L"hkLoadLibraryExA", (PVOID**)&originalLoadLibraryExA, hkLoadLibraryExA);
const hook GlobalHook_LoadLibraryExW(L"hkLoadLibraryExW", (PVOID**)&originalLoadLibraryExW, hkLoadLibraryExW);
const hook GlobalHook_FreeLibrary(L"hkFreeLibrary", (PVOID**)&originalFreeLibrary, hkFreeLibrary);
//const hook GlobalHook_MCCGetInput(L"hkMCCGetInput", L"MCC-Win64-Shipping.exe", mcc::function::OFFSET_MCCGETINPUT, (PVOID**)&originalMCCInput, hkMCCGetInput);

tas_hooks::tas_hooks()
{
	gRuntimePatches.push_back(RuntimePatch_EnableH1DevConsole);
	//gRuntimeHooks.push_back(RuntimeHook_Halo1HandleInput);
	gRuntimeHooks.push_back(RuntimeHook_Halo1GetNumberOfTicksToTick);
	//gRuntimeHooks.push_back(RuntimeHook_Halo2Tick);
	//gRuntimeHooks.push_back(RuntimeHook_Halo2TickLoop);
	//gRuntimeHooks.push_back(RuntimeHook_Halo3Tick);

	//gGlobalHooks.push_back(GlobalHook_D3D9SetRenderState);
	//gGlobalHooks.push_back(GlobalHook_D3D11RSSetState);
	gGlobalHooks.push_back(GlobalHook_D3D11Present);
	gGlobalHooks.push_back(GlobalHook_D3D11Draw);
	gGlobalHooks.push_back(GlobalHook_D3D11DrawIndexed);
	gGlobalHooks.push_back(GlobalHook_LoadLibraryA);
	gGlobalHooks.push_back(GlobalHook_LoadLibraryW);
	gGlobalHooks.push_back(GlobalHook_LoadLibraryExA);
	gGlobalHooks.push_back(GlobalHook_LoadLibraryExW);
	gGlobalHooks.push_back(GlobalHook_FreeLibrary);
	//gGlobalHooks.push_back(GlobalHook_MCCGetInput);

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
		originalD3D11RSSetState = (D3D11RSSetState_t)methods[104];
		originalD3D11Draw = (D3D11Draw_t)methods[74];
		originalD3D11DrawIndexed = (D3D11DrawIndexed_t)methods[73];
	}

	// D3D9 Hooking
	//if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success) {
	//	// See https://github.com/Rebzzel/kiero/blob/master/METHODSTABLE.txt for available functions
	//	auto methods = kiero::getMethodsTable();
	//	originalD3D9SetRenderState = (D3D9SetRenderState_t)methods[57];
	//}

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

	tas::overlay::set_wireframe();
	tas::overlay::render(InputCache);

	return originalD3D11Present(SwapChain, SyncInterval, Flags);
}

void hkD3D11RSSetState(ID3D11DeviceContext* Ctx, ID3D11RasterizerState* pRasterizerState)
{
	tas::overlay::set_wireframe(Ctx, 0);
}

void hkD3D9SetRenderState(IDirect3DDevice9* Device, D3DRENDERSTATETYPE State, DWORD Value)
{
	if (State == D3DRS_FILLMODE) {
		Value = D3DFILLMODE::D3DFILL_WIREFRAME;
	}
	Device->SetRenderState(State, Value);
	//originalD3D9SetRenderState(Device, State, Value);
}

void hkD3D11Draw(ID3D11DeviceContext* Ctx, UINT VertexCount, UINT StartVertexLocation)
{
	if (VertexCount > 1) {
		//tas::overlay::set_wireframe(Ctx);
	}
	originalD3D11Draw(Ctx, VertexCount, StartVertexLocation);
}

void hkD3D11DrawIndexed(ID3D11DeviceContext* Ctx, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	tas::overlay::set_wireframe(Ctx, IndexCount);
	
	if (tas::overlay::should_render(IndexCount)) {
		originalD3D11DrawIndexed(Ctx, IndexCount, StartIndexLocation, BaseVertexLocation);
	}

	tas::overlay::set_normal(Ctx);
}

struct input {
	uint8_t everything[520];
};

char hkHalo1HandleInput() {
	return originalHalo1HandleInput();
}

// The MCCGetInput function is used to pass input from the MCC parent process to each individual game DLL.
// Be careful when doing anything game-specific in here unless doing the proper checks!

bool globalStall = false;
bool recording = false;
bool playback = false;
bool forceTick = false;
int currentPlaybackFrame = 0;
uint8_t hkMCCGetInput(int64_t functionAddr, int64_t unknown, MCCInput* Input) {

	auto OriginalReturn = originalMCCInput(functionAddr, unknown, Input);

	bool tasEnabled = true;
	if (!tasEnabled) {
		return OriginalReturn;
	}

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

	tas::overlay::set_current_speedometer(speedo);

	// Halo 1
	auto H1DLL = dll_cache::get_module_handle(HALO1_DLL_WSTR);
	if (H1DLL.has_value()) {
		auto rng = value_ptr<int32_t>(H1DLL.value(), halo1::data::OFFSET_RNG);
		auto tick_ptr = value_ptr<int32_t>(H1DLL.value(), halo1::data::OFFSET_TICK_BASE, { 0xC });

		if (!rng || !tick_ptr) {
			return OriginalReturn;
		}

		if (globalStall) {
			return OriginalReturn;
		}

		int32_t tick = *tick_ptr;
		static int32_t lasttick = tick;
		static int32_t absoluteTick = 0;
		static bool tickRevert = false;

		tas_logger::info("GetInput on tick {} | RNG({})", tick, *rng);

		if (tick == 0 && recording) {
			InputCache.reset();
			InputCache.start_tick(*rng);
		}

		if (tick == 0) {
			absoluteTick = 0;
			currentPlaybackFrame = 0;
			speedo->clear();
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

	// Halo 2
	/*
	auto H2DLL = dll_cache::get_module_handle(HALO2_DLL_WSTR);
	if (H2DLL.has_value()) {

		auto tick_ptr = value_ptr<int32_t>(H2DLL.value(), halo2::data::OFFSET_TICK_BASE, { 0x8 });
		auto rng_ptr = value_ptr<int32_t>(H2DLL.value(), halo2::data::OFFSET_RNG, { 0x4 });

		if (!tick_ptr || !rng_ptr) {
			return OriginalReturn;
		}

		if (globalStall) {
			return OriginalReturn;
		}

		int32_t rng = *rng_ptr;
		int32_t tick = *tick_ptr;
		static int32_t lasttick = tick;
		static int32_t absoluteTick = 0;

		tas_logger::info("GetInputH2 on tick {} | RNG({})", tick, rng);

		if (tick == 0 && recording) {
			InputCache.reset();
			InputCache.start_tick(rng);
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
			InputCache.start_tick(rng);
		}

		if (recording) {
			InputCache.push_input(*Input);
		}

		if (ticked) {
			absoluteTick++;
			currentPlaybackFrame = 0;
		}

		if (playback && !GetAsyncKeyState(VK_ESCAPE)) {
			auto hasCurrentInput = InputCache.get_input(absoluteTick, currentPlaybackFrame, rng);
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
	*/
	// Halo 3
	/*
	auto H3DLL = dll_cache::get_module_handle(HALO3_DLL_WSTR);
	if (H3DLL.has_value()) {

		auto TLSIndex = value_ptr<uint32_t>(H3DLL.value(), halo3::data::OFFSET_TICK_TLSINDEX);
		int64_t* TLSResult = (int64_t*)(__readgsqword(0x58) + 8 * (int64_t)*TLSIndex);
		auto tick_ptr = value_ptr<int32_t>((void*)*TLSResult, 0x58, { 0xC });
		auto rng = value_ptr<int32_t>(H3DLL.value(), halo3::data::OFFSET_RNG);

		if (!tick_ptr) {
			return OriginalReturn;
		}

		int32_t tick = *tick_ptr;
		static int32_t lasttick = tick;
		static int32_t absoluteTick = 0;

		tas_logger::info("GetInputH3 | Tick[{}] | RNG[{}]", tick, *rng);

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
			absoluteTick++;
			currentPlaybackFrame = 0;
		}

		if (playback && !GetAsyncKeyState(VK_ESCAPE)) {
			auto hasCurrentInput = InputCache.get_input(absoluteTick, currentPlaybackFrame, *rng);
			if (hasCurrentInput.has_value()) {
				auto currentInput = hasCurrentInput.value();
				memcpy_s(Input, sizeof(MCCInput), &currentInput.input, sizeof(currentInput.input));
				currentPlaybackFrame++;

				if (tick == 0 && currentPlaybackFrame == 0) {
					*rng = currentInput.rng;
				}
			}
			else {
				currentPlaybackFrame = 0;
				tas_logger::info("Tick({}) No input", absoluteTick);
			}
		}

		// Just in case
		if (!playback) {
			forceTick = false;
		}

		lasttick = tick;
	}
	*/
	
	// DEFAULT
	return OriginalReturn;
}

int64_t hkH1GetNumberOfTicksToTick(float a1, uint8_t a2) {

	/*if (GetAsyncKeyState(VK_NUMPAD0)) {
		globalStall = false;
		tas_logger::warning("stall stopped");
	}*/
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

	if (originalReturn) {
		auto H1DLL = dll_cache::get_module_handle(HALO1_DLL_WSTR);
		auto currentCameraPosition = value_ptr<glm::vec3>(H1DLL.value(), 0x219AF08);
		speedo->push_back(*currentCameraPosition);
	}

	/*if (GetAsyncKeyState(VK_RIGHT)) {
		originalReturn = 1;
	}*/

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

int64_t hkH2Tick(void* a1)
{
	//tas_logger::info("        H2 Tick);
	return originalH2Tick(a1);
}

void hkH2TickLoop(int32_t a1, float* a2) {
	/*if (GetAsyncKeyState(VK_NUMPAD0)) {
		globalStall = false;
		tas_logger::warning("stall stopped");
	}*/
	if (globalStall) {
		return;
	}

	if (playback) {
		if (forceTick) {
			return originalH2TickLoop(1, a2);
		}
		else {
			return;
		}
	}

	if (a2 && *a2 > 0.0f) {
		//tas_logger::info("    H2 Tick Loop: {}/{}", a1, *a2);
	}

	return originalH2TickLoop(a1, a2);
}

void hkH3Tick()
{
	//tas_logger::info("    H3 Tick");
	originalH3Tick();
}

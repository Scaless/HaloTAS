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

std::once_flag gOverlayInitialized;
// Global hooks should only apply to areas that will never be unloaded
std::vector<hook> gGlobalHooks;
// Runtime hooks & patches are applied and unapplied when dlls are loaded and unloaded
std::vector<hook> gRuntimeHooks;
std::vector<patch> gRuntimePatches;
// Applied dynamically when game engine is loaded
std::vector<hook> gGameEngineHooks;

std::shared_ptr<speedometer> speedo = std::make_shared<speedometer>(5, 30);

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

typedef void (*D3D11RSSetState_t)(ID3D11DeviceContext* Ctx, ID3D11RasterizerState* pRasterizerState);
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

struct GameEngine
{
	void* vptr[32];
};

struct GameHost
{
	void* vptr[64];
};

typedef HANDLE (*GameEnginePlayGame_t)(void* engine, GameHost** host, void* options);
HANDLE hkGameEnginePlayGame(void* engine, GameHost** host, void* options);
GameEnginePlayGame_t engine_play_game_vptr;

typedef bool (*GameHostUpdateInput_t)(void* host, uint64_t param_a, MCCInput* input);
bool hkGameHostUpdateInput(void* host, uint64_t param_a, MCCInput* input);
GameHostUpdateInput_t originalGameHostUpdateInput;

typedef int64_t(*H1EngineCommand_t)(void* engine, const char* command);
H1EngineCommand_t engine_command_vptr;

void* current_game_host = nullptr;
void* current_engine = nullptr;
GameEngineType current_engine_type = GameEngineType::None;

typedef int64_t (*H1CreateGameEngine_t)(GameEngine*** out_engine_ptr);
int64_t hkHalo1CreateGameEngine(GameEngine*** out_engine_ptr);
H1CreateGameEngine_t originalH1CreateGameEngine;

typedef int64_t (*H2CreateGameEngine_t)(GameEngine*** out_engine_ptr);
int64_t hkHalo2CreateGameEngine(GameEngine*** out_engine_ptr);
H2CreateGameEngine_t originalH2CreateGameEngine;

typedef int64_t (*H3CreateGameEngine_t)(GameEngine*** out_engine_ptr);
int64_t hkHalo3CreateGameEngine(GameEngine*** out_engine_ptr);
H3CreateGameEngine_t originalH3CreateGameEngine;

//TODO: not hooked up, engine could be invalid in the menus
typedef void (*GameEngineDestructor_t)(GameEngine* engine);
void hkGameEngineDestructor(GameEngine* engine);
GameEngineDestructor_t originalGameEngineDestructor;

typedef uint64_t (*Halo1Tick_t)(int64_t param_a, int64_t param_b);
uint64_t hkHalo1Tick(int64_t param_a, int64_t param_b);
Halo1Tick_t originalHalo1Tick;

typedef void (*Halo1Tick2_t)(float delta);
void hkHalo1TickDelta(float delta);
Halo1Tick2_t originalHalo1Tick2;

//const hook RuntimeHook_Halo1HandleInput(L"hkHalo1HandleInput", HALO1_DLL_WSTR, halo1::function::OFFSET_H1_HANDLE_INPUT, (PVOID**)&originalHalo1HandleInput, hkHalo1HandleInput);
const hook RuntimeHook_Halo1GetNumberOfTicksToTick(L"hkH1GetNumberOfTicksToTick", HALO1_DLL_WSTR, halo1::function::OFFSET_H1_GET_NUMBER_OF_TICKS, (PVOID**)&originalH1GetNumberOfTicksToTick, hkH1GetNumberOfTicksToTick);
//const hook RuntimeHook_Halo2Tick(L"hkHalo2Tick", HALO2_DLL_WSTR, halo2::function::OFFSET_H2_TICK, (PVOID**)&originalH2Tick, hkH2Tick);
//const hook RuntimeHook_Halo2TickLoop(L"hkHalo2TickLoop", HALO2_DLL_WSTR, halo2::function::OFFSET_H2_TICK_LOOP, (PVOID**)&originalH2TickLoop, hkH2TickLoop);
//const hook RuntimeHook_Halo3Tick(L"hkHalo3Tick", HALO3_DLL_WSTR, halo3::function::OFFSET_H3_TICK, (PVOID**)&originalH3Tick, hkH3Tick);

const hook RuntimeHook_Halo1Tick(L"Halo1Tick", HALO1_DLL_WSTR, 0xc44320, (PVOID**)&originalHalo1Tick, hkHalo1Tick);
const hook RuntimeHook_Halo1Tick2(L"Halo1Tick2", HALO1_DLL_WSTR, 0x0ac19a0, (PVOID**)&originalHalo1Tick2, hkHalo1TickDelta);

const hook RuntimeHook_Halo1CreateGameEngine(L"hkH1CreateGameEngine", HALO1_DLL_WSTR, "CreateGameEngine", (PVOID**)&originalH1CreateGameEngine, hkHalo1CreateGameEngine);
const hook RuntimeHook_Halo2CreateGameEngine(L"hkH2CreateGameEngine", HALO2_DLL_WSTR, "CreateGameEngine", (PVOID**)&originalH2CreateGameEngine, hkHalo2CreateGameEngine);
const hook RuntimeHook_Halo3CreateGameEngine(L"hkH3CreateGameEngine", HALO3_DLL_WSTR, "CreateGameEngine", (PVOID**)&originalH3CreateGameEngine, hkHalo3CreateGameEngine);

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
	//gRuntimeHooks.push_back(RuntimeHook_Halo1HandleInput);
	//gRuntimeHooks.push_back(RuntimeHook_Halo1GetNumberOfTicksToTick);
	//gRuntimeHooks.push_back(RuntimeHook_Halo2Tick);
	//gRuntimeHooks.push_back(RuntimeHook_Halo2TickLoop);
	//gRuntimeHooks.push_back(RuntimeHook_Halo3Tick);

	gRuntimeHooks.push_back(RuntimeHook_Halo1Tick);
	gRuntimeHooks.push_back(RuntimeHook_Halo1Tick2);

	gRuntimeHooks.push_back(RuntimeHook_Halo1CreateGameEngine);
	gRuntimeHooks.push_back(RuntimeHook_Halo2CreateGameEngine);
	gRuntimeHooks.push_back(RuntimeHook_Halo3CreateGameEngine);

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
	detach_temporary_hooks();
}

void tas_hooks::execute_halo1_command(const std::string& command)
{
	if (!current_engine) {
		tas_logger::warning("Can't execute command. Current engine is null, did you launch HaloTAS before you launched Halo 1?\n");
		return;
	}
	if (current_engine_type != GameEngineType::Halo1) {
		tas_logger::warning("Can't execute command. Commands are only supported in Halo 1.\n");
		return;
	}
	std::string engine_command = "HS: " + command;
	engine_command_vptr(current_engine, engine_command.c_str());
}

GameEngineType tas_hooks::get_loaded_engine()
{
	if (!current_engine) {
		return GameEngineType::None;
	}
	return current_engine_type;
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

void tas_hooks::detach_temporary_hooks()
{
	for (auto& hook : gGameEngineHooks) {
		hook.detach();
	}
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

	//tas::overlay::set_wireframe();
	tas::overlay::render(InputCache);

	return originalD3D11Present(SwapChain, SyncInterval, Flags);
}

void hkD3D11RSSetState(ID3D11DeviceContext* Ctx, ID3D11RasterizerState* pRasterizerState)
{
	originalD3D11RSSetState(Ctx, pRasterizerState);
	//tas::overlay::set_wireframe(Ctx, 0);
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
	//tas::overlay::set_wireframe(Ctx, IndexCount);
	
	if (tas::overlay::should_render(IndexCount)) {
		originalD3D11DrawIndexed(Ctx, IndexCount, StartIndexLocation, BaseVertexLocation);
	}

	//tas::overlay::set_normal(Ctx);
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

// Stolen from BCS :)
constexpr size_t k_game_variant_buffer_size = 0x1CC00;
constexpr size_t k_map_variant_buffer_size = 0xE800;
struct s_peer_context
{
	long long secure_address;
};
struct s_player_context_v1
{
	long long xbox_user_id;
	long team;
	long player_assigned_team;
	long long secure_address;
};
struct s_player_context_v2 : public s_player_context_v1
{
	long __unknown18;
	long __unknown1C;
};
struct s_game_options_v3
{
public:
	bool visual_remaster : 1;
	bool music_remaster : 1;
	bool sfx_remaster : 1;
	bool is_host : 1;

	// SUCK IT!
	unsigned char launch_flags : 4;
private:
	char __padding0[3];
	char __padding1[6];
public:
	unsigned __int8 tick_length;
private:
	__int8 : 8;
public:
	long game_mode;
	long map_id;

	long campaign_difficulty_level;
private:
	char __padding2[4];
public:
	unsigned short campaign_insertion_point;
	short infinity_mission_id;
	unsigned long long launcher_skull_mask;

	s_peer_context party;
	s_peer_context local;

	s_peer_context peers[17];
	uint64_t peer_count;

	s_player_context_v2 players[16];
	uint64_t player_count;
	s_peer_context host;

	char game_variant_buffer[k_game_variant_buffer_size];
	char map_variant_buffer[k_map_variant_buffer_size];

	uint64_t game_state_header_size;
	char* game_state_header;
	const char* saved_film_path;
	wchar_t* custom_engine_name;
};

HANDLE hkGameEnginePlayGame(void* engine, GameHost** host, void* options) {

	s_game_options_v3* opt = static_cast<s_game_options_v3*>(options);

	tas_logger::info("hkGameEnginePlayGame");
	tas_logger::info("\tmap_id: {}", opt->map_id);
	tas_logger::info("\tcampaign_difficulty_level: {}", opt->campaign_difficulty_level);
	tas_logger::info("\tpeer_count: {}", opt->peer_count);
	tas_logger::info("\tplayer_count: {}", opt->player_count);

	HANDLE result = engine_play_game_vptr(engine, host, options);

	current_game_host = host;

	originalGameHostUpdateInput = static_cast<GameHostUpdateInput_t>((*host)->vptr[30]);

	hook RuntimeHook_Halo1GameHostUpdateInput(L"hkDynamicHalo1GameHostUpdateInput", (PVOID**)&originalGameHostUpdateInput, hkGameHostUpdateInput);
	RuntimeHook_Halo1GameHostUpdateInput.attach();
	gGameEngineHooks.push_back(RuntimeHook_Halo1GameHostUpdateInput);

	return result;
}

bool hkGameHostUpdateInput(void* host, uint64_t param_a, MCCInput* input) {
	tas_logger::info("hkGameHostUpdateInput");
	auto result = originalGameHostUpdateInput(host, param_a, input);
	return result;
}

int64_t hkHalo1CreateGameEngine(GameEngine*** out_engine_ptr)
{
	int64_t result = originalH1CreateGameEngine(out_engine_ptr);
	tas_logger::info("hkHalo1CreateGameEngine: Engine Created");

	current_engine = *out_engine_ptr;
	current_engine_type = GameEngineType::Halo1;

	originalGameEngineDestructor = static_cast<GameEngineDestructor_t>((**out_engine_ptr)->vptr[0]);
	engine_play_game_vptr = static_cast<GameEnginePlayGame_t>((**out_engine_ptr)->vptr[2]);
	engine_command_vptr = static_cast<H1EngineCommand_t>((**out_engine_ptr)->vptr[9]);

	hook RuntimeHook_Halo1GameEnginePlayGame(L"hkDynamicHalo1GameEnginePlayGame", (PVOID**)&engine_play_game_vptr, hkGameEnginePlayGame);
	RuntimeHook_Halo1GameEnginePlayGame.attach();
	gGameEngineHooks.push_back(RuntimeHook_Halo1GameEnginePlayGame);

	hook RuntimeHook_Halo1GameEngineDestructor(L"hkDynamicHalo1GameEngineDestructor", (PVOID**)&originalGameEngineDestructor, hkGameEngineDestructor);
	RuntimeHook_Halo1GameEngineDestructor.attach();
	gGameEngineHooks.push_back(RuntimeHook_Halo1GameEngineDestructor);

	return result;
}

int64_t hkHalo2CreateGameEngine(GameEngine*** out_engine_ptr)
{
	int64_t result = originalH2CreateGameEngine(out_engine_ptr);
	tas_logger::info("hkHalo2CreateGameEngine: Engine Created");

	originalGameEngineDestructor = static_cast<GameEngineDestructor_t>((**out_engine_ptr)->vptr[0]);
	engine_play_game_vptr = static_cast<GameEnginePlayGame_t>((**out_engine_ptr)->vptr[2]);

	hook RuntimeHook_Halo2GameEnginePlayGame(L"hkDynamicHalo2GameEnginePlayGame", (PVOID**)&engine_play_game_vptr, hkGameEnginePlayGame);
	RuntimeHook_Halo2GameEnginePlayGame.attach();
	gGameEngineHooks.push_back(RuntimeHook_Halo2GameEnginePlayGame);

	hook RuntimeHook_Halo2GameEngineDestructor(L"hkDynamicHalo2GameEngineDestructor", (PVOID**)&originalGameEngineDestructor, hkGameEngineDestructor);
	RuntimeHook_Halo2GameEngineDestructor.attach();
	gGameEngineHooks.push_back(RuntimeHook_Halo2GameEngineDestructor);

	current_engine = *out_engine_ptr;
	current_engine_type = GameEngineType::Halo2;
	engine_command_vptr = nullptr;

	return result;
}

int64_t hkHalo3CreateGameEngine(GameEngine*** out_engine_ptr)
{
	int64_t result = originalH3CreateGameEngine(out_engine_ptr);
	tas_logger::info("hkHalo3CreateGameEngine: Engine Created");

	originalGameEngineDestructor = static_cast<GameEngineDestructor_t>((**out_engine_ptr)->vptr[0]);
	engine_play_game_vptr = static_cast<GameEnginePlayGame_t>((**out_engine_ptr)->vptr[2]);

	hook RuntimeHook_Halo3GameEnginePlayGame(L"hkDynamicHalo3GameEnginePlayGame", (PVOID**)&engine_play_game_vptr, hkGameEnginePlayGame);
	RuntimeHook_Halo3GameEnginePlayGame.attach();
	gGameEngineHooks.push_back(RuntimeHook_Halo3GameEnginePlayGame);

	hook RuntimeHook_Halo3GameEngineDestructor(L"hkDynamicHalo3GameEngineDestructor", (PVOID**)&originalGameEngineDestructor, hkGameEngineDestructor);
	RuntimeHook_Halo3GameEngineDestructor.attach();
	gGameEngineHooks.push_back(RuntimeHook_Halo3GameEngineDestructor);

	current_engine = *out_engine_ptr;
	current_engine_type = GameEngineType::Halo3;
	engine_command_vptr = nullptr;

	return result;
}

void hkGameEngineDestructor(GameEngine* engine)
{
	tas_logger::info("hkGameEngineDestructor");
	originalGameEngineDestructor(engine);

	for (hook& hk : gGameEngineHooks) {
		hk.detach();
	}
	gGameEngineHooks.clear();

	current_engine = nullptr;
	current_engine_type = GameEngineType::None;
	engine_command_vptr = nullptr;
}

uint64_t hkHalo1Tick(int64_t param_a, int64_t param_b) {
	//tas_logger::info("H1Tick: {}, {}", param_a, param_b);
	return originalHalo1Tick(param_a, param_b);
}

void hkHalo1TickDelta(float delta) {
	//tas_logger::info("hkHalo1TickDelta: {}", delta);

	/*bool slomoKeyDown = GetAsyncKeyState(0x57) || GetAsyncKeyState(0x41) || GetAsyncKeyState(0x53) || GetAsyncKeyState(0x44) || GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RBUTTON);*/

	//originalHalo1Tick2(delta * (!slomoKeyDown ? .2f : 1.0f));
	originalHalo1Tick2(delta);
	
}

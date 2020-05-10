#include "tas_hooks.h"
#include "tas_logger.h"
#include "tas_input_handler.h"
#include "randomizer.h"
#include "halo_engine.h"
#include "render_d3d9.h"

// Function Defs
typedef HRESULT(__stdcall* GetDeviceState_t)(LPDIRECTINPUTDEVICE*, DWORD, LPVOID*);
typedef HRESULT(__stdcall* GetDeviceData_t)(LPDIRECTINPUTDEVICE*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
typedef void(__cdecl* SimulateTick_t)(int);
typedef char(__cdecl* AdvanceFrame_t)(float);
typedef int(__cdecl* BeginLoop_t)();
typedef void(__cdecl* GetMouseKeyboardGamepadInput_t)();
typedef void(__cdecl* AdvanceEffectsTimer_t)(float);
typedef HRESULT(__stdcall* D3D9BeginScene_t)(IDirect3DDevice9* pDevice);
typedef HRESULT(__stdcall* D3D9EndScene_t)(IDirect3DDevice9* pDevice);
typedef HRESULT(__stdcall* D3D9Present_t)(IDirect3DDevice9* pDevice, const RECT*, const RECT*, HWND, RGNDATA*);

// Declarations
HRESULT __stdcall hkGetDeviceState(LPDIRECTINPUTDEVICE* pDevice, DWORD cbData, LPVOID* lpvData);
HRESULT __stdcall hkGetDeviceData(LPDIRECTINPUTDEVICE* pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
void __cdecl hkSimulateTick(int);
char __cdecl hkAdvanceFrame(float);
int __cdecl hkBeginLoop();
void __cdecl hkGetMouseKeyboardGamepadInput();
void __cdecl hkAdvanceEffectsTimer(float);
HRESULT __stdcall hkD3D9EndScene(IDirect3DDevice9* pDevice);
HRESULT __stdcall hkD3D9BeginScene(IDirect3DDevice9* pDevice);
HRESULT __stdcall hkD3D9Present(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion);

// Store original pointers to old functions
GetDeviceState_t originalGetDeviceState;
GetDeviceData_t originalGetDeviceData;
SimulateTick_t originalSimulateTick = (SimulateTick_t)(0x45B780);
AdvanceFrame_t originalAdvanceFrame = (AdvanceFrame_t)(0x470BF0);
BeginLoop_t originalBeginLoop = (BeginLoop_t)(0x4C6E80);
AdvanceEffectsTimer_t originalAdvanceEffectsTimer = (AdvanceEffectsTimer_t)(0x45b4f0);
GetMouseKeyboardGamepadInput_t originalGetMouseKeyboardGamepadInput = (GetMouseKeyboardGamepadInput_t)(0x490760);
D3D9EndScene_t originalD3D9EndScene;
D3D9BeginScene_t originalD3D9BeginScene;
D3D9Present_t originalD3D9Present;

void tas_hooks::detours_error(LONG detourResult) {
	if (detourResult != NO_ERROR) {
		throw;
	}
}

void tas_hooks::hook_function(PVOID* originalFunc, PVOID replacementFunc)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	detours_error(DetourAttach(originalFunc, replacementFunc));
	detours_error(DetourTransactionCommit());
}

void tas_hooks::unhook_function(PVOID* originalFunc, PVOID replacementFunc)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(originalFunc, replacementFunc);
	DetourTransactionCommit();
}

void tas_hooks::hook_dinput8()
{
	// DIRECTINPUT8
	LPDIRECTINPUT8 pDI8;
	DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&pDI8, NULL);
	if (!pDI8) {
		tas_logger::fatal("Couldn't get dinput8 handle.");
		exit(1);
	}

	LPDIRECTINPUTDEVICE8 pDI8Dev = nullptr;
	pDI8->CreateDevice(GUID_SysKeyboard, &pDI8Dev, NULL);
	if (!pDI8Dev) {
		pDI8->Release();
		tas_logger::fatal("Couldn't create dinput8 device.");
		exit(1);
	}

	void** dinputVTable = *reinterpret_cast<void***>(pDI8Dev);

	originalGetDeviceState = (GetDeviceState_t)(dinputVTable[9]);
	originalGetDeviceData = (GetDeviceData_t)(dinputVTable[10]);

	pDI8Dev->Release();
	pDI8->Release();

	hook_function(&(PVOID&)originalGetDeviceState, hkGetDeviceState);
	hook_function(&(PVOID&)originalGetDeviceData, hkGetDeviceData);
}

void tas_hooks::hook_d3d9()
{
	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D) {
		tas_logger::fatal("Couldn't get d3d9 handle.");
		exit(1);
	}

	int mainMonitorX = GetSystemMetrics(SM_CXSCREEN);
	int mainMonitorY = GetSystemMetrics(SM_CYSCREEN);

	D3DPRESENT_PARAMETERS d3dpp = { 0 };
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetForegroundWindow();
	d3dpp.Windowed = TRUE;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = std::clamp(mainMonitorX, 800, 8000);
	d3dpp.BackBufferHeight = std::clamp(mainMonitorY, 600, 8000);

	IDirect3DDevice9* dummyD3D9Device = nullptr;
	pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dummyD3D9Device);
	if (!dummyD3D9Device)
	{
		pD3D->Release();
		tas_logger::fatal("Couldn't create dummy d3d9 device.");
		exit(1);
	}

	void** d3d9VTable = *reinterpret_cast<void***>(dummyD3D9Device);

#if _DEBUG
	// Breakpoint to find additional D3D functions if needed
	// Make sure symbols are loaded
	for (int i = 0; i < 180; i++) {
		auto funcPtr = d3d9VTable[i];
	}
#endif

	originalD3D9BeginScene = (D3D9BeginScene_t)(d3d9VTable[41]);
	hook_function(&(PVOID&)originalD3D9BeginScene, hkD3D9BeginScene);

	originalD3D9EndScene = (D3D9EndScene_t)(d3d9VTable[42]);
	hook_function(&(PVOID&)originalD3D9EndScene, hkD3D9EndScene);

	originalD3D9Present = (D3D9Present_t)(d3d9VTable[17]);
	hook_function(&(PVOID&)originalD3D9Present, hkD3D9Present);

	dummyD3D9Device->Release();
	pD3D->Release();
}

void tas_hooks::hook_halo_engine()
{
	hook_function(&(PVOID&)originalSimulateTick, hkSimulateTick);
	hook_function(&(PVOID&)originalAdvanceFrame, hkAdvanceFrame);
	hook_function(&(PVOID&)originalBeginLoop, hkBeginLoop);
	hook_function(&(PVOID&)originalGetMouseKeyboardGamepadInput, hkGetMouseKeyboardGamepadInput);
	hook_function(&(PVOID&)originalAdvanceEffectsTimer, hkAdvanceEffectsTimer);
}

void tas_hooks::attach_all()
{
	hook_dinput8();
	hook_d3d9();
	hook_halo_engine();
}

void tas_hooks::detach_all()
{
	unhook_function(&(PVOID&)originalGetDeviceState, hkGetDeviceState);
	unhook_function(&(PVOID&)originalGetDeviceData, hkGetDeviceData);
	unhook_function(&(PVOID&)originalSimulateTick, hkSimulateTick);
	unhook_function(&(PVOID&)originalAdvanceFrame, hkAdvanceFrame);
	unhook_function(&(PVOID&)originalBeginLoop, hkBeginLoop);
	unhook_function(&(PVOID&)originalGetMouseKeyboardGamepadInput, hkGetMouseKeyboardGamepadInput);
	unhook_function(&(PVOID&)originalAdvanceEffectsTimer, hkAdvanceEffectsTimer);
	unhook_function(&(PVOID&)originalD3D9EndScene, hkD3D9EndScene);
	unhook_function(&(PVOID&)originalD3D9BeginScene, hkD3D9BeginScene);
	unhook_function(&(PVOID&)originalD3D9Present, hkD3D9Present);
}

/* HOOKED FUNCTIONS */

static int32_t pressedMouseTick = 0;
static bool btn_down[3] = { false, false, false };
static bool btn_queued[3] = { false, false, false };
HRESULT __stdcall hkGetDeviceState(LPDIRECTINPUTDEVICE* lpDevice, DWORD cbData, LPVOID* lpvData) // Mouse
{
	HRESULT hResult = DI_OK;
	hResult = originalGetDeviceState(lpDevice, cbData, lpvData);
	return hResult;
}

static bool tab_down = false;
static bool enter_down = false;
static bool g_down = false;
static DWORD lastsequence = 0;
static int32_t pressedTick = 0;
static bool enterPreviousFrame = false;
static int32_t enterPressedFrame = 0;
static bool queuedEnter = false;
static bool queuedTab = false;
static bool queuedG = false;
HRESULT __stdcall hkGetDeviceData(LPDIRECTINPUTDEVICE* pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) // Keyboard
{
	HRESULT hResult = DI_OK;

	auto& inputHandler = tas_input_handler::get();

	if (inputHandler.this_tick_enter() && pressedTick != inputHandler.get_current_playback_tick()) {
		queuedEnter = true;
	}
	if (inputHandler.this_tick_tab() && pressedTick != inputHandler.get_current_playback_tick()) {
		queuedTab = true;
	}
	if (inputHandler.this_tick_g() && pressedTick != inputHandler.get_current_playback_tick()) {
		queuedG = true;
	}

	pressedTick = inputHandler.get_current_playback_tick();

	if (queuedEnter) {
		if (enter_down) {
			rgdod->dwData = 0x80;
		}
		else {
			rgdod->dwData = 0x00;
			queuedEnter = false;
		}
		rgdod->dwOfs = DIK_RETURN;
		rgdod->dwTimeStamp = GetTickCount();
		rgdod->dwSequence = ++lastsequence;
		enter_down = !enter_down;
		return hResult;
	}

	if (queuedTab) {
		if (tab_down) {
			rgdod->dwData = 0x80;
		}
		else {
			rgdod->dwData = 0x00;
			queuedTab = false;
		}
		rgdod->dwOfs = DIK_TAB;
		rgdod->dwTimeStamp = GetTickCount();
		rgdod->dwSequence = ++lastsequence;
		tab_down = !tab_down;
		return hResult;
	}

	if (queuedG) {
		if (g_down) {
			rgdod->dwData = 0x80;
		}
		else {
			rgdod->dwData = 0x00;
			queuedG = false;
		}
		rgdod->dwOfs = DIK_G;
		rgdod->dwTimeStamp = GetTickCount();
		rgdod->dwSequence = ++lastsequence;
		g_down = !g_down;
		return hResult;
	}

	hResult = originalGetDeviceData(pDevice, cbObjectData, rgdod, pdwInOut, dwFlags);
	return hResult;
}

void hkSimulateTick(int ticksAfterThis) {
	auto& gInputHandler = tas_input_handler::get();
	auto& gRandomizer = randomizer::get();
	auto& gEngine = halo_engine::get();

	gRandomizer.pre_tick();
	gInputHandler.pre_tick();
	originalSimulateTick(ticksAfterThis);
	gInputHandler.post_tick();
	gEngine.post_tick();
}

char hkAdvanceFrame(float deltaTime) {
	auto& gInputHandler = tas_input_handler::get();
	auto& gEngine = halo_engine::get();

	gEngine.pre_frame();
	gInputHandler.pre_frame();
	char c = originalAdvanceFrame(deltaTime);
	gInputHandler.post_frame();

	return c;
}

int __cdecl hkBeginLoop() {
	auto& gInputHandler = tas_input_handler::get();
	auto& gRandomizer = randomizer::get();
	auto& gEngine = halo_engine::get();

	gRandomizer.pre_loop();
	gInputHandler.pre_loop();
	auto ret = originalBeginLoop();
	gInputHandler.post_loop();

	return ret;
}

void __cdecl hkGetMouseKeyboardGamepadInput() {
	originalGetMouseKeyboardGamepadInput();
}

void __cdecl hkAdvanceEffectsTimer(float dt) {
	originalAdvanceEffectsTimer(dt);
}

HRESULT __stdcall hkD3D9BeginScene(IDirect3DDevice9* pDevice)
{
	return originalD3D9BeginScene(pDevice);
}

HRESULT __stdcall hkD3D9Present(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, RGNDATA* pDirtyRegion)
{
	auto& gEngine = halo_engine::get();
	if (gEngine.is_present_enabled()) {
		return originalD3D9Present(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	}

	return D3D_OK;
}

HRESULT __stdcall hkD3D9EndScene(IDirect3DDevice9* pDevice)
{
	auto& d3d9 = render_d3d9::get();
	d3d9.render(pDevice);

	return originalD3D9EndScene(pDevice);
}

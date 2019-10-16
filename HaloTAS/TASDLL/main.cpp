
#define DIRECTINPUT_VERSION 0x0800


#include <chrono>
#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <dinput.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <detours/detours.h>

#include "halo_engine.h"
#include "tas_overlay.h"
#include "tas_info_window.h"
#include "tas_input_handler.h"
#include "tas_options.h"
#include "tas_logger.h"
#include "livesplit.h"
#include "helpers.h"
#include "render_d3d9.h"
#include "hotkeys.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib")

typedef HRESULT(__stdcall* GetDeviceState_t)(LPDIRECTINPUTDEVICE*, DWORD, LPVOID *);
typedef HRESULT(__stdcall* GetDeviceData_t)(LPDIRECTINPUTDEVICE*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
typedef void(__cdecl* SimulateTick_t)(int);
typedef char(__cdecl* AdvanceFrame_t)(float);
typedef int(__cdecl* BeginLoop_t)();
typedef void(__cdecl* GetMouseKeyboardGamepadInput_t)();
typedef void(__cdecl* AdvanceEffectsTimer_t)(float);
typedef HRESULT(__stdcall* D3D9BeginScene_t)(IDirect3DDevice9* pDevice);
typedef HRESULT(__stdcall* D3D9EndScene_t)(IDirect3DDevice9* pDevice);

HRESULT __stdcall hkGetDeviceState(LPDIRECTINPUTDEVICE *pDevice, DWORD cbData, LPVOID *lpvData);
HRESULT __stdcall hkGetDeviceData(LPDIRECTINPUTDEVICE *pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
void __cdecl hkSimulateTick(int);
char __cdecl hkAdvanceFrame(float);
int __cdecl hkBeginLoop();
void __cdecl hkGetMouseKeyboardGamepadInput();
void __cdecl hkAdvanceEffectsTimer(float);
HRESULT __stdcall hkD3D9EndScene(IDirect3DDevice9* pDevice);
HRESULT __stdcall hkD3D9BeginScene(IDirect3DDevice9* pDevice);

GetDeviceState_t originalGetDeviceState;
GetDeviceData_t originalGetDeviceData;
SimulateTick_t originalSimulateTick = (SimulateTick_t)(0x45B780);
AdvanceFrame_t originalAdvanceFrame = (AdvanceFrame_t)(0x470BF0);
BeginLoop_t originalBeginLoop = (BeginLoop_t)(0x4C6E80);
AdvanceEffectsTimer_t originalAdvanceEffectsTimer = (AdvanceEffectsTimer_t)(0x45b4f0);
GetMouseKeyboardGamepadInput_t originalGetMouseKeyboardGamepadInput = (GetMouseKeyboardGamepadInput_t)(0x490760);
D3D9EndScene_t originalD3D9EndScene;
D3D9BeginScene_t originalD3D9BeginScene;

void detours_error(LONG detourResult) {
	if (detourResult != NO_ERROR) {
		throw;
	}
}

void AttachFunc(PVOID* originalFunc, PVOID replacementFunc) {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	detours_error(DetourAttach(originalFunc, replacementFunc));
	detours_error(DetourTransactionCommit());
}

void DetachFunc(PVOID* originalFunc, PVOID replacementFunc) {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(originalFunc, replacementFunc);
	DetourTransactionCommit();
}

std::unique_ptr<tas_info_window> infoWindow{ nullptr };

void run() {

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		tas_logger::fatal("Failed to initialize GLFW");
		return;
	}

	auto& gEngine = halo_engine::get();
	auto& gInputHandler = tas_input_handler::get();
	auto& gHotkeys = hotkeys::get();
	
	gHotkeys.load_hotkeys();
	gInputHandler.get_inputs_from_files();

	auto liveSplit = std::make_unique<livesplit>();
	auto overlay = std::make_unique<tas_overlay>();
	infoWindow = std::make_unique<tas_info_window>();

	auto lastEngineUpdate = std::chrono::system_clock::now();
	auto lastDisplayUpdate = std::chrono::system_clock::now();
	bool close = false;

	// Main loop
	while (!close)
	{
		glfwPollEvents();

		auto now = std::chrono::system_clock::now();
		auto engineUpdateDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEngineUpdate);
		if (engineUpdateDuration > std::chrono::seconds(10)) {
			gEngine.update_window_handle();
			lastEngineUpdate = now;
		}

		// Keep people honest
		auto hudUpdateDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDisplayUpdate);
		if (hudUpdateDuration > std::chrono::seconds(60)) {
			gEngine.print_hud_text(L"Official runs are invalid while HaloTAS is running!");
			lastDisplayUpdate = now;
		}

		livesplit_export newExport = {};
		strcpy_s((char*)&newExport.currentMap, sizeof(newExport.currentMap), halo::addr::MAP_STRING);
		liveSplit->update_export(newExport);

		auto input = infoWindow->getInput();
		if (input.loadPlayback) {
			gInputHandler.get_inputs_from_files();
		}
		if (input.overlayClose) {
			close |= true;
		}

		infoWindow->render();
		overlay->render(input.overlayOptions);

		gInputHandler.set_record(input.record);
		gInputHandler.set_playback(input.playback);

		close |= infoWindow->shouldClose();
	}

}

void attach_hooks() {

	DetourRestoreAfterWith();

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

	// DIRECTX9
	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D) {
		tas_logger::fatal("Couldn't get d3d9 handle.");
		exit(1);
	}

	D3DPRESENT_PARAMETERS d3dpp = { 0 };
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetForegroundWindow();
	d3dpp.Windowed = TRUE;

	IDirect3DDevice9* dummyD3D9Device = nullptr;
	pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &dummyD3D9Device);
	if (!dummyD3D9Device)
	{
		pD3D->Release();
		return;
	}

	void** d3d9VTable = *reinterpret_cast<void***>(dummyD3D9Device);

#if _DEBUG
	// Breakpoint to find additional D3D functions if needed
	// Make sure symbols are loaded
	for (int i = 0; i < 180; i++) {
		auto funcPtr = d3d9VTable[i];
	}
#endif

	originalD3D9BeginScene = (D3D9EndScene_t)(d3d9VTable[41]);
	AttachFunc(&(PVOID&)originalD3D9BeginScene, hkD3D9BeginScene);

	originalD3D9EndScene = (D3D9EndScene_t)(d3d9VTable[42]);
	AttachFunc(&(PVOID&)originalD3D9EndScene, hkD3D9EndScene);

	dummyD3D9Device->Release();
	pD3D->Release();

	AttachFunc(&(PVOID&)originalGetDeviceState, hkGetDeviceState);
	AttachFunc(&(PVOID&)originalGetDeviceData, hkGetDeviceData);
	AttachFunc(&(PVOID&)originalSimulateTick, hkSimulateTick);
	AttachFunc(&(PVOID&)originalAdvanceFrame, hkAdvanceFrame);
	AttachFunc(&(PVOID&)originalBeginLoop, hkBeginLoop);
	AttachFunc(&(PVOID&)originalGetMouseKeyboardGamepadInput, hkGetMouseKeyboardGamepadInput);
	AttachFunc(&(PVOID&)originalAdvanceEffectsTimer, hkAdvanceEffectsTimer);
}

void detach_hooks() {
	DetachFunc(&(PVOID&)originalGetDeviceState, hkGetDeviceState);
	DetachFunc(&(PVOID&)originalGetDeviceData, hkGetDeviceData);
	DetachFunc(&(PVOID&)originalSimulateTick, hkSimulateTick);
	DetachFunc(&(PVOID&)originalAdvanceFrame, hkAdvanceFrame);
	DetachFunc(&(PVOID&)originalBeginLoop, hkBeginLoop);
	DetachFunc(&(PVOID&)originalGetMouseKeyboardGamepadInput, hkGetMouseKeyboardGamepadInput);
	DetachFunc(&(PVOID&)originalAdvanceEffectsTimer, hkAdvanceEffectsTimer);
	DetachFunc(&(PVOID&)originalD3D9EndScene, hkD3D9EndScene);
	DetachFunc(&(PVOID&)originalD3D9BeginScene, hkD3D9BeginScene);
}

DWORD WINAPI Main_Thread(HMODULE hDLL)
{
	// Make sure folder exists to store HaloTAS files
	std::filesystem::create_directory("HaloTASFiles");

	tas_logger::info("===== HaloTAS Started =====");
	tas_logger::info("Current working directory: %s", std::filesystem::current_path().c_str());

	// Wait for halo.exe to init otherwise we may access invalid memory
	while (*halo::addr::SIMULATION_TICK <= 0) {
		
	}
	
	attach_hooks();

	run();

	glfwTerminate();
	detach_hooks();
	tas_logger::info("===== HaloTAS Closed =====");
	FreeLibraryAndExitThread(hDLL, NULL);
}

// Entry point for DLL
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID /*Reserved*/)
{
	DWORD dwThreadID;
	if (Reason == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)Main_Thread, hDLL, 0, &dwThreadID);
	}
	else if (Reason == DLL_PROCESS_DETACH) {
		//MessageBox(NULL, "Detach", "", MB_OK);
	}
	return TRUE;
}

static int32_t pressedMouseTick = 0;

static bool btn_down[3] = { false, false, false };
static bool btn_queued[3] = {false, false, false };

HRESULT __stdcall hkGetDeviceState(LPDIRECTINPUTDEVICE *lpDevice, DWORD cbData, LPVOID *lpvData) // Mouse
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
static bool queuedEnter = false;
static bool queuedTab = false;
static bool queuedG = false;
HRESULT __stdcall hkGetDeviceData(LPDIRECTINPUTDEVICE *pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) // Keyboard
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

	if (queuedTab){
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

	gInputHandler.pre_tick();
	originalSimulateTick(ticksAfterThis);
	gInputHandler.post_tick();
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

HRESULT __stdcall hkD3D9EndScene(IDirect3DDevice9* pDevice)
{
	auto& d3d9 = render_d3d9::get();
	d3d9.render(pDevice);

	return originalD3D9EndScene(pDevice);
}

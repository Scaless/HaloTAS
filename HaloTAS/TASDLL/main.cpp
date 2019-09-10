
#define DIRECTINPUT_VERSION 0x0800


#include <chrono>
#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <dinput.h>

#include "halo_engine.h"
#include "tas_overlay.h"
#include "tas_info_window.h"
#include "tas_input_handler.h"
#include "livesplit.h"
#include "helpers.h"
#include "detours.h"

typedef HRESULT(__stdcall* GetDeviceState_t)(LPDIRECTINPUTDEVICE, DWORD, LPVOID *);
typedef HRESULT(__stdcall* GetDeviceData_t)(LPDIRECTINPUTDEVICE, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
typedef void(__cdecl* SimulateTick_t)(int);
typedef char(__cdecl* AdvanceFrame_t)(float);

HRESULT __stdcall hkGetDeviceState(LPDIRECTINPUTDEVICE pDevice, DWORD cbData, LPVOID *lpvData);
HRESULT __stdcall hkGetDeviceData(LPDIRECTINPUTDEVICE pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
void __cdecl hkSimulateTick(int);
char __cdecl hkAdvanceFrame(float);

// 0x8150 - WIN10 17134
// 0x8CC0 - WIN10 18362
DWORD GetDeviceStateOffset = 0x8CC0; // Offset from DirectInput8Create to CDIDev_GetDeviceState

// 0x7F90 - WIN10 17134
// 0x8B10 - WIN10 18362
DWORD GetDeviceDataOffset = 0x8B10; // Offset from DirectInput8Create to CDIDev_GetDeviceData

void add_log(const char* format, ...);

GetDeviceState_t originalGetDeviceState;
GetDeviceData_t originalGetDeviceData;
SimulateTick_t originalSimulateTick = (SimulateTick_t)(0x45B780);
AdvanceFrame_t originalAdvanceFrame = (AdvanceFrame_t)(0x470BF0);

void run() {

	auto liveSplit = std::make_unique<livesplit>();
	auto overlay = std::make_unique<tas_overlay>();
	auto infoWindow = std::make_unique<tas_info_window>();

	auto& gEngine = halo_engine::get();
	auto& gInputHandler = tas_input_handler::get();
	gInputHandler.get_inputs_from_files();

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
		strcpy_s((char*)&newExport.currentMap, sizeof(newExport.currentMap), ADDR_MAP_STRING);
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


DWORD WINAPI Main_Thread(HMODULE hDLL)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		return 1;
	}

	// Main execution happens in run() so that destructors are called properly
	// before FreeLibraryAndExitThread gets called
	run();

	glfwTerminate();

	FreeLibraryAndExitThread(hDLL, NULL);
}

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

// Entry point for DLL
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID /*Reserved*/)
{
	DWORD dwThreadID;
	if (Reason == DLL_PROCESS_ATTACH) {
		add_log("==========LOG START==========");

		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)Main_Thread, hDLL, 0, &dwThreadID);

		DetourRestoreAfterWith();

		// Get Handles to necessary functions
		HMODULE hDInput8 = GetModuleHandle("dinput8.dll");
		if (hDInput8 == NULL) {
			add_log("Couldn't get handle for dinput8.dll.");
			return 0;
		}
		FARPROC dwDirectInput8Create = GetProcAddress(hDInput8, "DirectInput8Create");
		if (dwDirectInput8Create == NULL) {
			add_log("GetProcAddress failed to find DirectInput8Create.");
			return 0;
		}

		originalGetDeviceState = (GetDeviceState_t)((DWORD)dwDirectInput8Create + GetDeviceStateOffset);
		originalGetDeviceData = (GetDeviceData_t)((DWORD)dwDirectInput8Create + GetDeviceDataOffset);

		AttachFunc(&(PVOID&)originalGetDeviceState, hkGetDeviceState);
		AttachFunc(&(PVOID&)originalGetDeviceData, hkGetDeviceData);
		AttachFunc(&(PVOID&)originalSimulateTick, hkSimulateTick);
		AttachFunc(&(PVOID&)originalAdvanceFrame, hkAdvanceFrame);
	}
	else if (Reason == DLL_PROCESS_DETACH) {
		DetachFunc(&(PVOID&)originalGetDeviceState, hkGetDeviceState);
		DetachFunc(&(PVOID&)originalGetDeviceData, hkGetDeviceData);
		DetachFunc(&(PVOID&)originalSimulateTick, hkSimulateTick);
		DetachFunc(&(PVOID&)originalAdvanceFrame, hkAdvanceFrame);
	}
	return TRUE;
}

//Creates a Logfile in the Game Directory
void add_log(const char* format, ...)
{
	HANDLE filehandle;
	DWORD dwReadBytes;
	char buffer[2048];
	char writebuffer[2048];
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, format, args);
	filehandle = CreateFile("HaloTASLog.txt", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
	SetFilePointer(filehandle, 0, 0, FILE_END);
	sprintf_s(writebuffer, 2048, "Log Added: %s\r\n", buffer);
	WriteFile(filehandle, writebuffer, strlen(writebuffer), &dwReadBytes, 0);
	CloseHandle(filehandle);
}

static bool tab_down = true;
static bool tab_toggle = true;
static int tab_toggle_count = 0;

HRESULT __stdcall hkGetDeviceState(LPDIRECTINPUTDEVICE lpDevice, DWORD cbData, LPVOID *lpvData) // Mouse
{
	HRESULT hResult = DI_OK;

	hResult = originalGetDeviceState(lpDevice, cbData, lpvData);

	return hResult;
}

static DWORD lastsequence = 0;

HRESULT __stdcall hkGetDeviceData(LPDIRECTINPUTDEVICE pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) // Keyboard
{
	HRESULT hResult = DI_OK;

	hResult = originalGetDeviceData(pDevice, cbObjectData, rgdod, pdwInOut, dwFlags);
	return hResult;

	
	
	///////////////////////////////////////////////////

	/*DWORD cbObjectDataFake = cbObjectData;
	LPDIDEVICEOBJECTDATA rgdodFake = rgdod;
	LPDWORD pdwInOutFake = pdwInOut;
	DWORD dwFlagsFake = dwFlags;*/


	//add_log("Data: dwOfs: %x | dwData: %x | dwSeq: %x | dwTS: %x | hResult: %x | uAppData: %x",
	//	rgdod->dwOfs, rgdod->dwData, rgdod->dwSequence, rgdod->dwTimeStamp, hResult, rgdod->uAppData);
	//
	//// No Input
	//if (true) {
	//	rgdod->dwOfs = 0;
	//	rgdod->dwData = 0x2;
	//	rgdod->dwSequence = 0x19db40;
	//	rgdod->dwTimeStamp = 0x1902987a;
	//	rgdod->uAppData = 0x0019fe48;
	//}
	//
	//return hResult;

	//////////////////////////////////////////////////
	//if (rgdod == nullptr) {
	//	return hResult;
	//}

	//if (tab_toggle){
	//	if (tab_down) {
	//		rgdod->dwData = 0x80;
	//	}
	//	else {
	//		rgdod->dwData = 0x00;
	//		tab_toggle = false;
	//	}
	//	rgdod->dwOfs = DIK_RETURN;
	//	rgdod->dwTimeStamp = GetTickCount();
	//	rgdod->dwSequence = ++lastsequence;
	//	tab_down = !tab_down;
	//}
	//else {

	//	//hResult = originalGetDeviceData(pDevice, cbObjectDataFake, rgdodFake, pdwInOutFake, dwFlagsFake);
	//	hResult = originalGetDeviceData(pDevice, cbObjectData, rgdod, pdwInOut, dwFlags);

	//	/*if (rgdod->dwOfs == 0) {
	//		rgdod->dwOfs = 0;
	//		rgdod->dwData = 0x2;
	//		rgdod->dwSequence = 0x19db40;
	//		rgdod->dwTimeStamp = 0x1902987a;
	//		rgdod->uAppData = 0x0019fe48;
	//	}*/
	//
	//	lastsequence = rgdod->dwSequence;
	//
	//	tab_toggle = true;

	//	tab_toggle_count++;
	//	if (tab_toggle_count > 30) {
	//		tab_toggle_count = 0;
	//	}
	//}

	//return hResult;
}



void hkSimulateTick(int ticksAfterThis) {

	auto& gInputHandler = tas_input_handler::get();

	// Pre-Tick
	gInputHandler.pre_tick();

	originalSimulateTick(ticksAfterThis);

	// Post-Tick
	gInputHandler.post_tick();

}

char hkAdvanceFrame(float deltaTime) {

	// Pre-Frame

	char c = originalAdvanceFrame(deltaTime);

	// Post-Frame

	return c;
}

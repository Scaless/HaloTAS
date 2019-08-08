
#define DIRECTINPUT_VERSION 0x0800


#include <chrono>
#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <dinput.h>

#include "globals.h"
#include "halo_engine.h"
#include "tas_overlay.h"
#include "tas_info_window.h"
#include "tas_input_handler.h"
#include "livesplit.h"
#include "helpers.h"
#include "detours.h"

typedef HRESULT(__stdcall* GetDeviceState_t)(LPDIRECTINPUTDEVICE, DWORD, LPVOID *);
HRESULT __stdcall hkGetDeviceState(LPDIRECTINPUTDEVICE pDevice, DWORD cbData, LPVOID *lpvData);
DWORD GetDeviceStateOffset = 0x8150; // This is the offset of GetDeviceState from DInput8.dll
// Open IDA and Import the DInput8.dll, then look in the Functions Table for DirectInput8Create
// There is an Address (1000XXXX or 0CXXXXX) - copy it and save it for later
// Then take a look for CDIDev_GetDeviceState and copy that address too
// Now substract the Address from CDIDev_GetDeviceState from DIrectInput8Create and u'll get your offset

typedef HRESULT(__stdcall* GetDeviceData_t)(LPDIRECTINPUTDEVICE, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
HRESULT __stdcall hkGetDeviceData(LPDIRECTINPUTDEVICE pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
DWORD GetDeviceDataOffset = 0x7F90;

HANDLE tmpHandle = NULL;
HMODULE hModDInput8 = NULL;
DWORD dwGetDeviceState = NULL;
DWORD dwGetDeviceData = NULL;
FARPROC dwDirectInput8Create = NULL;

struct MyKeys
{
	BYTE Key;
	DWORD StartTime;
	DWORD TTL;
	BOOLEAN isDown;
};
MyKeys KeyBuffer[256];

DWORD WINAPI HookThread();
void add_log(const char* format, ...);
void SendKeyDInput(byte DIK_, DWORD time);
GetDeviceState_t pGetDeviceState;
GetDeviceData_t pGetDeviceData;

uint32_t reviverRNG = 1'975'045'979;
uint32_t KeyesRNGSeed = 4'112'291'920;

void exportKeyesRNG() {
	std::ofstream outfile;
	outfile.open("keyes_rng.csv", std::ios::app);
	int32_t currentRNG = (int32_t)KeyesRNGSeed;

	for (int i = 0; i < 100'000; i++) {
		outfile << currentRNG << "," << rng_to_double(currentRNG) << std::endl;
		currentRNG = next_rng(currentRNG);
	}
	outfile.close();
}

// Entry point when compiled as EXE
int main() {
	exportKeyesRNG();
}

void run() {
	gEngine = std::make_unique<halo_engine>();
	gInputHandler = std::make_unique<tas_input_handler>();

	auto liveSplit = std::make_unique<livesplit>();
	auto overlay = std::make_unique<tas_overlay>();
	auto infoWindow = std::make_unique<tas_info_window>();

	gEngine->initialize();
	gInputHandler->get_inputs_from_files();

	auto lastEngineUpdate = std::chrono::system_clock::now();
	auto lastDisplayUpdate = std::chrono::system_clock::now();
	bool close = false;

	// Main loop
	while (!close)
	{
		glfwPollEvents();

		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEngineUpdate) > std::chrono::milliseconds(10000)) {
			gEngine->update_window_handle();
			lastEngineUpdate = now;
		}

		// Keep people honest
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDisplayUpdate) > std::chrono::seconds(20)) {
			//gEngine->print_hud_text(L"Official runs are invalid while HaloTAS is running!");
			lastDisplayUpdate = now;
		}

		livesplit_export newExport = {};
		strcpy_s((char*)&newExport.currentMap, sizeof(newExport.currentMap), ADDR_MAP_STRING);
		liveSplit->update_export(newExport);

		auto input = infoWindow->getInput();
		if (input.loadPlayback) {
			gInputHandler->get_inputs_from_files();
		}
		if (input.overlayClose) {
			close |= true;
		}

		infoWindow->render();
		overlay->render(input.overlayOptions);

		gInputHandler->set_record(input.record);
		gInputHandler->set_playback(input.playback);

		close |= infoWindow->shouldClose();
	}

	gEngine->cleanup();
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

	TerminateThread(tmpHandle, 0);
	CloseHandle(tmpHandle);

	FreeLibraryAndExitThread(hDLL, NULL);
}

// Entry point for DLL
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID /*Reserved*/)
{
	DWORD dwThreadID;
	if (Reason == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)Main_Thread, hDLL, 0, &dwThreadID);

		add_log("==========LOG START==========");
		add_log("DLL Attached");

		DetourRestoreAfterWith();

		while (!hModDInput8)
		{
			add_log("Searching dinput8.dll...");
			hModDInput8 = GetModuleHandle("dinput8.dll");
			Sleep(100);
		}
		add_log("Found dinput8.dll: %x !", hModDInput8);

		while (!dwDirectInput8Create)
		{
			add_log("Searching GetDeviceState...");
			dwDirectInput8Create = GetProcAddress(hModDInput8, "DirectInput8Create");
			Sleep(100);
		}
		add_log("Found DirectInput8Create: %x !", dwDirectInput8Create);

		pGetDeviceState = (GetDeviceState_t)((DWORD)dwDirectInput8Create + GetDeviceStateOffset);

		add_log("GetDeviceState is here (DirectInput8Create - %x): %x", GetDeviceStateOffset, dwGetDeviceState);
		add_log("Hooking GetDeviceState...");
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		LONG detourResultState = DetourAttach(&(PVOID&)pGetDeviceState, hkGetDeviceState);
		DetourTransactionCommit();

		if (detourResultState != NO_ERROR) {
			switch (detourResultState) {
			case(ERROR_INVALID_BLOCK):
				add_log("Detours failed: ERROR_INVALID_BLOCK");
				return 0;
			case(ERROR_INVALID_HANDLE):
				add_log("Detours failed: ERROR_INVALID_HANDLE");
				return 0;
			case(ERROR_INVALID_OPERATION):
				add_log("Detours failed: ERROR_INVALID_OPERATION");
				return 0;
			case(ERROR_NOT_ENOUGH_MEMORY):
				add_log("Detours failed: ERROR_NOT_ENOUGH_MEMORY");
				return 0;
			default:
				break;
			}
		}

		pGetDeviceData = (GetDeviceData_t)((DWORD)dwDirectInput8Create + GetDeviceDataOffset);

		add_log("GetDeviceData is here (DirectInput8Create - %x): %x", GetDeviceDataOffset, dwGetDeviceData);
		add_log("Hooking GetDeviceData...");
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		LONG detourResultData = DetourAttach(&(PVOID&)pGetDeviceData, hkGetDeviceData);
		DetourTransactionCommit();
		add_log("Initiate Keyboard Buffer...");

		if (detourResultData != NO_ERROR) {
			switch (detourResultData) {
			case(ERROR_INVALID_BLOCK):
				add_log("Detours failed: ERROR_INVALID_BLOCK");
				return 0;
			case(ERROR_INVALID_HANDLE):
				add_log("Detours failed: ERROR_INVALID_HANDLE");
				return 0;
			case(ERROR_INVALID_OPERATION):
				add_log("Detours failed: ERROR_INVALID_OPERATION");
				return 0;
			case(ERROR_NOT_ENOUGH_MEMORY):
				add_log("Detours failed: ERROR_NOT_ENOUGH_MEMORY");
				return 0;
			default:
				break;
			}
		}

		add_log("Creating Thread...");
		tmpHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&HookThread, 0, 0, 0);
		if (!tmpHandle)
		{
			add_log("ThreadCreation Failed!");
		}
	}
	else if (Reason == DLL_PROCESS_DETACH) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)pGetDeviceState, hkGetDeviceState);
		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)pGetDeviceData, hkGetDeviceData);
		DetourTransactionCommit();
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
	filehandle = CreateFile("Log.txt", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
	SetFilePointer(filehandle, 0, 0, FILE_END);
	sprintf_s(writebuffer, 2048, "Log Added: %s\r\n", buffer);
	WriteFile(filehandle, writebuffer, strlen(writebuffer), &dwReadBytes, 0);
	CloseHandle(filehandle);
}

static bool tab_down = true;
static bool tab_toggle = true;
static int tab_toggle_count = 0;

DWORD WINAPI HookThread()
{
	add_log("Thread Created");

	//initiate buffer
	for (int i = 0; i < 256; i++)
	{
		KeyBuffer[i].isDown = false;
		KeyBuffer[i].Key = 0;
		KeyBuffer[i].StartTime = 0;
		KeyBuffer[i].TTL = 0;
	}

	add_log("Going into Main Loop...");
	while (true)
	{
		if (GetAsyncKeyState(VK_F11) & 1 << 15)
		{
			// We check the Most Sigificant Bit from VK_F5 (F5) whilst we shifted it with 15 bits to left 1 
			// and then a small delay so we have enaught time to release the key
			add_log("F11 pushed attempting to sendkey");
			// Now we send a A Key with 1 sec time to our Game
			//SendKeyDInput(DIK_TAB, 1000);
		}
	}

	return 0;
}
void SendKeyDInput(byte DIK, DWORD time)
{
	KeyBuffer[DIK].Key = DIK;
	KeyBuffer[DIK].TTL = time;
	KeyBuffer[DIK].StartTime = GetTickCount();
}

HRESULT __stdcall hkGetDeviceState(LPDIRECTINPUTDEVICE lpDevice, DWORD cbData, LPVOID *lpvData) // Mouse
{
	HRESULT hResult = DI_OK;

	hResult = pGetDeviceState(lpDevice, cbData, lpvData);

	return hResult;
}

static DWORD lastsequence = 0;

HRESULT __stdcall hkGetDeviceData(LPDIRECTINPUTDEVICE pDevice, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) // Keyboard
{
	HRESULT hResult = DI_OK;

	static BYTE buffer[256];
	int key_count = 0;
	for (int i = 0; i < 256; i++)
	{
		if (KeyBuffer[i].Key != 0 && KeyBuffer[i].TTL > 0 && KeyBuffer[i].StartTime != 0)
		{
			if (GetTickCount() > KeyBuffer[i].StartTime + KeyBuffer[i].TTL && KeyBuffer[i].isDown)
			{
				KeyBuffer[i].Key = 0;
				KeyBuffer[i].StartTime = 0;
				KeyBuffer[i].TTL = 0;
				KeyBuffer[i].isDown = false;
				buffer[KeyBuffer[i].Key] = 0;
			}
			else {
				KeyBuffer[i].isDown = true;
				buffer[KeyBuffer[i].Key] = 0x80;
				key_count += 1;
				add_log("Sending Key %x for %i milliseconds count: %i", KeyBuffer[i].Key, KeyBuffer[i].TTL, key_count);
			}
		}
	}

	if (tab_toggle){
		//hResult = pGetDeviceData(pDevice, cbObjectData, rgdod, pdwInOut, dwFlags);
		if (tab_down) {
			rgdod->dwData = 0x80;
		}
		else {
			rgdod->dwData = 0x00;
			tab_toggle = false;
		}
		rgdod->dwOfs = DIK_ESCAPE;
		rgdod->dwTimeStamp = GetTickCount();
		rgdod->dwSequence = ++lastsequence;
		tab_down = !tab_down;
	}
	else {
		hResult = pGetDeviceData(pDevice, cbObjectData, rgdod, pdwInOut, dwFlags);
		if (rgdod->dwOfs != 0) {
			lastsequence = rgdod->dwSequence;
		}
		tab_toggle_count++;
		if (tab_toggle_count > 30) {
			tab_toggle = true;
			tab_toggle_count = 0;
		}
	}

	add_log("Data: %x | %x | %x", rgdod->dwOfs, rgdod->dwData, rgdod->dwSequence);


	return hResult;
}
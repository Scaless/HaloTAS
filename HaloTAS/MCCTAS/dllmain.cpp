// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include <memory>

#include "global.h"
#include "windows_utilities.h"
#include "windows_console.h"
#include "tas_hooks.h"
#include "halo_constants.h"
#include "gui_interop.h"
#include "dll_cache.h"

// Main Execution Loop
void RealMain() {
	acquire_global_unhandled_exception_handler();

	auto consoleWindow = std::make_unique<windows_console>();
	consoleWindow->open();

	const std::string startupMessage = "MCCTAS Started!\r\n"
		"To close MCCTAS, press CTRL + C while this window is focused. MCC will continue running. "
		"If you accidentally click into this window and highlight something, just press ESC to unfreeze the game.\r\n\r\n"
		"MCCTAS significantly modifies game behavior and is not approved for official speedruns.\r\n"
		"If you see this console window open, MCCTAS is running.\r\n\r\n"
		"Report bugs and suggest features on Github at: https://github.com/Scaless/HaloTAS. \r\n"
		"Have fun! ~Scales"
		;
	tas_logger::warning("{}", startupMessage);

	dll_cache::initialize();

	auto interop = std::make_unique<gui_interop>();
	auto hooks = std::make_unique<tas_hooks>();
	hooks->attach_all();

	// Most of the cool stuff happens in other threads.
	// This loop is just to keep stuff alive.
	while (!global::is_kill_set()) {
		Sleep(100);
	}

	hooks->detach_all();

	tas_logger::info("MCCTAS Stopped!");
	tas_logger::flush_and_exit();
	release_global_unhandled_exception_handler();
}

// This thread is created by the dll when loaded into the process, see RealMain() for the actual event loop.
// Do NOT put any allocations in this function because the call to FreeLibraryAndExitThread()
// will occur before they fall out of scope and will not be cleaned up properly! This is very
// important for being able to hotload the DLL multiple times without restarting the game.
DWORD WINAPI MainThread(HMODULE hDLL) {
	RealMain();

	Sleep(200);
	FreeLibraryAndExitThread(hDLL, NULL);
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	DWORD dwThreadID;

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, &dwThreadID);
	}

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

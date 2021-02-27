// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "global.h"
#include "windows_utilities.h"
#include "windows_console.h"
#include "tas_hooks.h"
#include "gui_interop.h"
#include "dll_cache.h"
#include "signatures.h"

// Main Execution Loop
void RealMain() {
	acquire_global_unhandled_exception_handler();

	auto consoleWindow = std::make_unique<windows_console>();
	consoleWindow->open();

	const std::string startupMessage = "MCCTAS Started!\r\n"
		"To close MCCTAS, press CTRL + C while this window is focused. MCC will continue running.\r\n"
		"If you accidentally highlight something in this window, focus this window and press ESC to unfreeze the game.\r\n"
		"Press the tilde(~) key in-game to open the MCCTAS console. Enter the 'help' command for more information on using the console.\r\n\r\n"
		"MCCTAS significantly modifies game behavior and is not approved for official speedruns.\r\n"
		"Report bugs and suggest features on Github at: https://github.com/Scaless/HaloTAS. \r\n"
		"Have fun!\r\n~Scales\r\n"
		;

	tas_logger::warning("{}", startupMessage);
	tas_logger::warning("KNOWN ISSUES: \r\n\t Changing graphics options crashes the game.");

	dll_cache::initialize();

	auto interop = std::make_unique<gui_interop>();
	auto hooks = std::make_unique<tas_hooks>();
	hooks->attach_all();

	//const char* save_text = "non compressed save";
	//void* checkpoint_copy = new std::byte[0x4BA014];
	//memcpy(checkpoint_copy, save_text, 0x14);
	//memcpy((std::byte*)checkpoint_copy + 0x14, (void*)0x1D892460000, 0x4BA000);
	
	//memcpy((void*)0x1D802049084, (void*)0x1D892460000, 0x4BA000);
	//printf("%p", checkpoint_copy);

	//memcpy((void*)0x2678A420000, (void*)0x7FF444FA2024, 0x4BA000);
	//memset((void*)0x7FFD94DBFC2C, 0, 1);

	// Most of the cool stuff happens in other threads.
	// This loop is just to keep stuff alive.
	while (!global::is_kill_set()) {
		Sleep(100);
	}

	hooks->detach_all();

	//delete checkpoint_copy;

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
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
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

// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include <memory>

#include "windows_utilities.h"
#include "windows_console.h"
#include "tas_hooks.h"
#include "halo_constants.h"
#include "gui_interop.h"

// Main Execution Loop
void RealMain() {

    auto interop = std::make_unique<gui_interop>();
    auto consoleWindow = std::make_unique<windows_console>();
    auto hooks = std::make_unique<tas_hooks>();
    //hooks->attach_all();

    while (!consoleWindow->get_exit_status()) {
        // Do stuff :)
        Sleep(100);
    }

    //hooks->detach_all();
}

// This thread is created by the dll when loaded into the process, see RealMain() for the actual event loop.
// Do NOT put any allocations in this function because the call to FreeLibraryAndExitThread()
// will occur before they fall out of scope and will not be cleaned up properly! This is very
// important for being able to hotload the DLL multiple times without restarting the game.
DWORD WINAPI MainThread(HMODULE hDLL) {
    RealMain();

    Sleep(1000);
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

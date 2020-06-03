// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include <memory>

#include "windows_utilities.h"
#include "windows_console.h"
#include "tas_hooks.h"
#include "halo_constants.h"
#include "gui_interop.h"

bool print_dlls() {

    std::vector<loaded_dll_info> dlls = 
    {
        loaded_dll_info(L"halo1.dll"),
        loaded_dll_info(L"haloreach.dll") 
    };

    fill_loaded_dlls_info(dlls);

    for(auto& dll : dlls) {
        wprintf(L"%s: \r\n", dll.name.c_str());
        wprintf(L"\tBase Address = %p\r\n", dll.info.lpBaseOfDll);
        wprintf(L"\tEntry Point = %p\r\n", dll.info.EntryPoint);
        wprintf(L"\tImage Size = %d bytes\r\n", dll.info.SizeOfImage);
    }
    
    return true;
}

// Main Execution Loop
void RealMain() {

    /*char* halo1DLLAddress = reinterpret_cast<char*>(0x180000000);
    char** halo1RuntimePtr = reinterpret_cast<char**>(halo1DLLAddress + 0x218E250);
    char* halo1RuntimeBase = reinterpret_cast<char*>(*halo1RuntimePtr);

    int32_t* tick = reinterpret_cast<int32_t*>(halo1RuntimeBase + 0x2F4);
    char* InputArray = reinterpret_cast<char*>(halo1DLLAddress + 0x218E2B0);*/

    auto interop = std::make_unique<gui_interop>();
    interop->start_pipe_server();

    auto consoleWindow = std::make_unique<windows_console>();
    
    auto hooks = std::make_unique<tas_hooks>();
    //hooks->attach_all();

    while (!consoleWindow->get_exit_status()) {
        consoleWindow->clear();
        print_dlls();

        //pipe_shit();

        Sleep(100);
    }

    interop->stop_pipe_server();

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

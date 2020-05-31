// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include <iostream>
#include <windows.h>

#include "windows_utilities.h"
#include "windows_console.h"
#include "tas_hooks.h"
#include "halo_constants.h"

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

void pipe_shit() {
    static int i = 0;
    i++;

    LPCWSTR lpszPipeName = TEXT("\\\\.\\pipe\\MCCTAS");
    CHAR chReadBuf[1024];
    DWORD cbRead;
    BOOL fResult;

    std::string Message = "Test " + std::to_string(i) + "\n";

    char writeBuffer[256];
    ZeroMemory(&writeBuffer, 256);

    Message.copy(&writeBuffer[1], Message.length());
    DWORD c = Message.length() + 1;
    writeBuffer[0] = (char)c;

    fResult = CallNamedPipe(
        lpszPipeName,          // pipe name 
        &writeBuffer,          // message to server 
        c,                     // message length 
        chReadBuf,             // buffer to receive reply 
        sizeof(chReadBuf),     // size of read buffer 
        &cbRead,               // number of bytes read 
        NMPWAIT_WAIT_FOREVER); // wait;-) 
}

DWORD WINAPI MainThread(HMODULE hDLL) {
    auto consoleWindow = std::make_unique<windows_console>();

    char* halo1DLLAddress = reinterpret_cast<char*>(0x180000000);
    char** halo1RuntimePtr = reinterpret_cast<char**>(halo1DLLAddress + 0x218E250);
    char* halo1RuntimeBase = reinterpret_cast<char*>(*halo1RuntimePtr);

    int32_t* tick = reinterpret_cast<int32_t*>(halo1RuntimeBase + 0x2F4);
    char* InputArray = reinterpret_cast<char*>(halo1DLLAddress + 0x218E2B0);

    int32_t lastTick = 0;
    while (!consoleWindow->get_exit_status()) {
        print_dlls();
        consoleWindow->clear();

        pipe_shit();

        if (*tick != lastTick) {
            InputArray[to_underlying(halo1::COMMON_KEYS::S)] = lastTick % 2;
            lastTick = *tick;
        }
        
        Sleep(10);
    }

    Sleep(1000);
    FreeLibraryAndExitThread(hDLL, NULL);
}

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

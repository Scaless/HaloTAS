// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include <iostream>
#include <windows.h>
#include <atomic>
#include <vector>

#include "windows_utilities.h"
#include "tas_hooks.h"

// https://stackoverflow.com/a/5866648
void clear_screen(char fill = ' ') {
    COORD tl = { 0,0 };
    CONSOLE_SCREEN_BUFFER_INFO s;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console, &s);
    DWORD written, cells = s.dwSize.X * s.dwSize.Y;
    FillConsoleOutputCharacter(console, fill, cells, tl, &written);
    FillConsoleOutputAttribute(console, s.wAttributes, cells, tl, &written);
    SetConsoleCursorPosition(console, tl);
}

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

std::atomic_bool exitFlag = false;

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch (CEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        exitFlag = true;
        FreeConsole();
        break;
    }
    return TRUE;
}

void init() {
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);

    
}
void deinit() {
    
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
    init();

    auto tasHooks = std::make_unique<tas_hooks>();
    
    tasHooks->attach_all(NULL, hDLL);
    
    while (!exitFlag) {
        print_dlls();
        Sleep(1000);
        clear_screen();
        pipe_shit();
    }

    tasHooks->detach_all();

    deinit();
    Sleep(3000);
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


// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include <tchar.h>
#include <Psapi.h>
#include <iostream>
#include <windows.h>
#include <atomic>


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
    HMODULE hMods[1024];
    HANDLE hProcess = GetCurrentProcess();
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];
            
            if (GetModuleBaseName(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
                //_tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]);
            }

            auto gameDllNames = { L"halo1.dll", L"haloreach.dll" };

            for (auto& dllName : gameDllNames) {
                if (!wcscmp(szModName, dllName)) {

                    MODULEINFO lpModInfo = {};
                    if (GetModuleInformation(hProcess, hMods[i], &lpModInfo, sizeof(lpModInfo))) {
                        wprintf(L"%s: \r\n", szModName);
                        printf("\tbase address = %p\r\n", lpModInfo.lpBaseOfDll);
                    }
                }
            }
        }
    }

    return true;
}

std::atomic_bool exitFlag = false;

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    char mesg[128];

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

DWORD WINAPI MainThread(HMODULE hDLL) {
    init();
    
    while (!exitFlag) {
        print_dlls();
        Sleep(1000);
        clear_screen();
    }

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


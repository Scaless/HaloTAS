#include "windows_console.h"
#include <iostream>

std::atomic_bool windows_console::mExitFlag = false;

BOOL WINAPI console_event_handler(DWORD CEvent)
{
    switch (CEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        windows_console::set_exit_status(true);
        FreeConsole();
        break;
    }
    return TRUE;
}

windows_console::windows_console()
{
    init();
}

windows_console::~windows_console()
{
    free_console();
}

void windows_console::init()
{
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)console_event_handler, TRUE);
}

void windows_console::free_console()
{
    if (stdout) {
        fclose(stdout);
    }
    FreeConsole();
}

void windows_console::clear()
{
    // https://stackoverflow.com/a/5866648
    char fill = ' ';
    COORD tl = { 0,0 };
    CONSOLE_SCREEN_BUFFER_INFO s;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console, &s);
    DWORD written, cells = s.dwSize.X * s.dwSize.Y;
    FillConsoleOutputCharacter(console, fill, cells, tl, &written);
    FillConsoleOutputAttribute(console, s.wAttributes, cells, tl, &written);
    SetConsoleCursorPosition(console, tl);
}

bool windows_console::get_exit_status()
{
    return mExitFlag;
}

void windows_console::set_exit_status(bool newStatus)
{
    windows_console::mExitFlag = newStatus;
}

#include "windows_console.h"
#include <iostream>
#include "global.h"

std::atomic_bool windows_console::mExitFlag = false;

BOOL WINAPI console_event_handler(DWORD CEvent)
{
	switch (CEvent)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		windows_console::set_exit_status(true);
		FreeConsole();
		global::kill_mcctas();
		break;
	}
	return TRUE;
}

windows_console::windows_console()
{

}

windows_console::~windows_console()
{
	free_console();
}

void windows_console::open()
{
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)console_event_handler, TRUE);

	// Stupid hack to remove the close button on the console window, otherwise the whole game will close
	// https://stackoverflow.com/a/12015131
	HWND hwnd = GetConsoleWindow();
	if (hwnd != NULL)
	{
		HMENU hMenu = GetSystemMenu(hwnd, FALSE);
		if (hMenu != NULL) {
			DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
		}
	}
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

bool windows_console::is_open()
{
	return mExitFlag;
}

void windows_console::set_exit_status(bool newStatus)
{
	windows_console::mExitFlag = newStatus;
}

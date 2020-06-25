#pragma once
#include <windows.h>
#include <atomic>

class windows_console
{
private:
	static std::atomic_bool mExitFlag;

public:
	windows_console();
	~windows_console();

	void init();
	void free_console();
	void clear();
	static bool get_exit_status();
	static void set_exit_status(bool newStatus);
	
};


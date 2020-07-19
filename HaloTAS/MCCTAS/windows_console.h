#pragma once
#include "pch.h"

class windows_console
{
private:
	static std::atomic_bool mExitFlag;

public:
	windows_console();
	~windows_console();

	void open();
	void free_console();
	void clear();
	static bool is_open();
	static void set_exit_status(bool newStatus);

};


#pragma once
#include <string>
#include <Psapi.h>
#include <vector>

struct loaded_dll_info {
	std::wstring name;
	MODULEINFO info = {};

	loaded_dll_info(std::wstring _name) : name(_name)
	{

	}
};

void fill_loaded_dlls_info(std::vector<loaded_dll_info>& dlls);



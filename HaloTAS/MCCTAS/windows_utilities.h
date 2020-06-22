#pragma once
#include "pch.h"
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

void patch_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size);

//std::vector<char> dll_create_snapshot(std::wstring dllName);
//void dll_restore_snapshot(std::wstring, const std::vector<char>& data);

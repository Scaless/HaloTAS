#pragma once
#include "pch.h"
#include <string>
#include <Psapi.h>
#include <vector>

void patch_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size);

std::wstring str_to_wstr(const std::string str);

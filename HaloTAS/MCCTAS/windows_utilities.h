#pragma once
#include "pch.h"

void patch_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size);

std::wstring str_to_wstr(const std::string str);

void acquire_global_unhandled_exception_handler();
void release_global_unhandled_exception_handler();

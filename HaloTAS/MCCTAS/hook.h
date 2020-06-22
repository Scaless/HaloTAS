#pragma once
#include "pch.h"

class hook {
private:
	PVOID* original_function;
	PVOID replaced_function;
public:
	hook(PVOID* old_func, PVOID new_func) : original_function(old_func), replaced_function(new_func)
	{
	}
	~hook() = default;

	void attach();
	void detach();
};


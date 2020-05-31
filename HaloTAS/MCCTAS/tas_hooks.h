#pragma once

#include "pch.h"

class tas_hooks
{
private:
	bool mD3D9HooksInstalled = false;

public:
	tas_hooks() = default;
	~tas_hooks() = default;

	void attach_all();
	void detach_all();

private:
	void detours_error(LONG detourResult);
	void hook_function(PVOID* originalFunc, PVOID replacementFunc);
	void unhook_function(PVOID* originalFunc, PVOID replacementFunc);
};


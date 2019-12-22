#pragma once

#include <Windows.h>
#include <dinput.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <detours/detours.h>

class tas_hooks
{
public:
	tas_hooks() = default;
	~tas_hooks() = default;

	void attach_all();
	void detach_all();
private:
	void detours_error(LONG detourResult);
	void hook_function(PVOID* originalFunc, PVOID replacementFunc);
	void unhook_function(PVOID* originalFunc, PVOID replacementFunc);

	void hook_dinput8();
	void hook_d3d9();
	void hook_halo_engine();
};

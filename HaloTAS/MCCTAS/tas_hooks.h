#pragma once

#include "pch.h"
#include <vector>

struct patch {
	bool applied = false;
	void* address = nullptr;
	std::vector<uint8_t> patch_data;
	std::vector<uint8_t> original_data;
};

struct hook {
	PVOID* original_function;
	PVOID replaced_function;

	hook(PVOID* old_func, PVOID new_func) : original_function(old_func), replaced_function(new_func)
	{
	}
};

class tas_hooks
{
private:
	std::vector<hook> mHooks;
	std::vector<patch> mPatches;

	const LPCWSTR H1DLLPATH = TEXT("../../../halo1/halo1.dll");
	HMODULE modH1DLL;
	const LPCWSTR H2DLLPATH = TEXT("../../../halo2/halo2.dll");
	HMODULE modH2DLL;
	const LPCWSTR H3DLLPATH = TEXT("../../../halo3/halo3.dll");
	HMODULE modH3DLL;
	const LPCWSTR REACHDLLPATH = TEXT("../../../haloreach/haloreach.dll");
	HMODULE modReachDLL;
	// TODO-SCALES: Verify these when games are released
	const LPCWSTR ODSTDLLPATH = TEXT("../../../halo3odst/halo3odst.dll");
	HMODULE modODSTDLL;
	const LPCWSTR H4DLLPATH = TEXT("../../../halo4/halo4.dll");
	HMODULE modH4DLL;
	/////////////////////////////////////////////////////

public:
	tas_hooks();
	~tas_hooks();

	void generate_hooks();
	void generate_patches();
	void attach_all();
	void detach_all();

private:

	// Increments/Decrements reference count to the halo game dlls in order
	// to prevent them from being unloaded when the current played game changes.
	// Required to have consistent patching without complex re-hooking when
	// DLLs are loaded and unloaded.
	void ref_halo_dlls();
	void deref_halo_dlls();

	// Patching Functions
	void apply_patch(patch& p);
	void restore_patch(patch& p);

	// Hooking Functions
	void detours_error(LONG detourResult);
	void hook_function(PVOID* originalFunc, PVOID replacementFunc);
	void unhook_function(PVOID* originalFunc, PVOID replacementFunc);
};


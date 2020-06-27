#include "hook.h"
#include <detours/detours.h>

void detours_error(LONG detourResult) {
	if (detourResult != NO_ERROR) {
		throw;
	}
}

void hook::detour_install(PVOID* old_func, PVOID new_func)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	detours_error(DetourAttach(old_func, new_func));
	detours_error(DetourTransactionCommit());
}

PVOID* hook::get_address_from_offset()
{
	HMODULE moduleBase = GetModuleHandle(mModuleName.c_str());
	if (!moduleBase) {
		mInstalled = false;
		return nullptr;
	}

	return (PVOID*)((uint8_t*)moduleBase + mOffset);
}

PVOID* hook::get_address_from_function_name()
{
	HMODULE moduleBase = GetModuleHandle(mModuleName.c_str());
	if (!moduleBase) {
		return nullptr;
	}
	return (PVOID*)GetProcAddress(moduleBase, mFunctionName.c_str());
}

void* hook::original_func_ptr()
{
	return (void*)mOriginalFunction;
}

void hook::attach()
{
	if (mInstalled) {
		return;
	}
	if (!mReplacedFunction) {
		return;
	}

	switch (mInstallType) {
	case hook_type::DIRECT:
	{
		detour_install(mOriginalFunction, mReplacedFunction);
		break;
	}
	case hook_type::MODULE_OFFSET:
	{
		mOriginalFunction = get_address_from_offset();
		if (!mOriginalFunction) {
			return;
		}
		detour_install(mOriginalFunction, mReplacedFunction);
		break;
	}
	case hook_type::MODULE_FUNCTION:
	{
		mOriginalFunction = get_address_from_function_name();
		if (!mOriginalFunction) {
			return;
		}
		detour_install((PVOID*)&mOriginalFunction, mReplacedFunction);
		break;
	}
	default:
	{
		tas_logger::warning(L"Failed to install hook with invalid install type ({})", to_underlying(mInstallType));
		return;
	}
	}

	mInstalled = true;
}

void hook::detach()
{
	// Don't try to detach when we know it's not installed
	if (!mInstalled) {
		return;
	}

	PVOID* original_func = mOriginalFunction ? mOriginalFunction : get_address_from_offset();
	if (!original_func || !mReplacedFunction) {
		return;
	}

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(original_func, mReplacedFunction);
	DetourTransactionCommit();
}

std::wstring_view hook::module_name()
{
	return mModuleName;
}

std::string_view hook::function_name()
{
	return mFunctionName;
}

#pragma once
#include "pch.h"

class hook {
private:
	enum class hook_type { DIRECT, MODULE_OFFSET, MODULE_FUNCTION };

	bool mInstalled = false;
	std::wstring mHookName;
	std::wstring mModuleName;
	std::string mFunctionName;
	int64_t mOffset = 0;
	PVOID** mOriginalFunction = nullptr;
	PVOID mReplacedFunction = nullptr;
	hook_type mInstallType;

public:
	hook(const std::wstring& hook_name, PVOID** original_func, PVOID new_func)
		: mHookName(hook_name), mOriginalFunction(original_func), mReplacedFunction(new_func), mInstallType(hook_type::DIRECT)
	{
	}
	hook(const std::wstring& hook_name, const std::wstring& moduleName, int64_t offset, PVOID** original_func, PVOID new_func)
		: mHookName(hook_name), mOffset(offset), mModuleName(moduleName), mOriginalFunction(original_func),
		mReplacedFunction(new_func), mInstallType(hook_type::MODULE_OFFSET)
	{
	}
	hook(const std::wstring& hook_name, const std::wstring& moduleName, const std::string& functionName, PVOID** original_func, PVOID new_func)
		: mHookName(hook_name), mModuleName(moduleName), mFunctionName(functionName), mOriginalFunction(original_func),
		mReplacedFunction(new_func), mInstallType(hook_type::MODULE_FUNCTION)
	{
	}
	~hook() = default;

	void attach();
	void detach();

	const std::wstring& hook_name();
	const std::wstring& module_name();
	const std::string& function_name();

private:
	void detour_install(PVOID* old_func, PVOID new_func);
	PVOID* get_address_from_offset();
	PVOID* get_address_from_function_name();
};


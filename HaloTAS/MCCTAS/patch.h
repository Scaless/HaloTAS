#pragma once
#include "pch.h"

class patch {
private:
	std::wstring mPatchName;
	std::wstring mModuleName;
	int64_t mOffset = 0;
	std::vector<uint8_t> mPatchData;
	std::vector<uint8_t> mOriginalData;

public:
	patch(const std::wstring& patch_name, const std::wstring& module, int64_t offset, const std::vector<uint8_t>& patch)
		: mPatchName(patch_name), mOffset(offset), mPatchData(patch), mModuleName(module)
	{
	}
	~patch() = default;

	void apply();
	void restore();

	const std::wstring& patch_name();
	const std::wstring& module_name();
};


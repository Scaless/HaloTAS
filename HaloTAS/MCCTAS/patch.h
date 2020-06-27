#pragma once
#include "pch.h"
#include <vector>

class patch {
private:
	std::wstring mModuleName;
	int64_t mOffset = 0;
	std::vector<uint8_t> mPatchData;
	std::vector<uint8_t> mOriginalData;

public:
	patch(const std::wstring& module, int64_t offset, const std::vector<uint8_t>& patch)
		: mOffset(offset), mPatchData(patch), mModuleName(module)
	{
	}
	~patch() = default;

	void apply();
	void restore();

	std::wstring_view module_name();
};


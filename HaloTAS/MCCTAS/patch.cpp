#include "patch.h"
#include "windows_utilities.h"

void patch::apply()
{
	HMODULE moduleBase = GetModuleHandle(mModuleName.c_str());
	if (!moduleBase) {
		return;
	}
	
	uint8_t* patchAddr = (uint8_t*)moduleBase + mOffset;
	size_t patchSize = mPatchData.size();
	
	// Get copy of original data
	mOriginalData.resize(patchSize);
	memcpy_s(mOriginalData.data(), mOriginalData.capacity(), patchAddr, patchSize);

	// Overwrite original with patch
	patch_memory(patchAddr, mPatchData.data(), patchSize);
}

void patch::restore()
{
	HMODULE moduleBase = GetModuleHandle(mModuleName.c_str());
	if (!moduleBase) {
		return;
	}
	uint8_t* patchAddr = (uint8_t*)moduleBase + mOffset;
	patch_memory(patchAddr, mOriginalData.data(), mOriginalData.size());
}

std::wstring_view patch::module_name()
{
	return mModuleName;
}

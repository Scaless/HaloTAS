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

	tas_logger::debug(L"Installed patch: {}", mPatchName);
}

void patch::restore()
{
	HMODULE moduleBase = GetModuleHandle(mModuleName.c_str());
	if (!moduleBase) {
		return;
	}
	uint8_t* patchAddr = (uint8_t*)moduleBase + mOffset;
	patch_memory(patchAddr, mOriginalData.data(), mOriginalData.size());

	tas_logger::debug(L"Uninstalled patch: {}", mPatchName);
}

const std::wstring& patch::patch_name()
{
	return mPatchName;
}

const std::wstring& patch::module_name()
{
	return mModuleName;
}

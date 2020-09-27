#pragma once
#include "pch.h"
#include "halo_constants.h"
#include "halo_types.h"
#include "dll_cache.h"

template <typename T>
class signature
{
	const std::string mSignatureName;
	const std::wstring mModuleName;
	std::vector<int16_t> mScanBytes;
	int mValueOffset;
	bool mCacheEnabled;
	mutable T* mCachedValue;

public:
	/// <summary>
	/// 
	/// </summary>
	/// <param name="signature_name">Name of the data at this signature</param>
	/// <param name="module_name">Name of the DLL that this sig resides in</param>
	/// <param name="scan_bytes">Byte array to scan for</param>
	/// <param name="value_offset">Offset after the start of the scan bytes that the desired value is at</param>
	/// <param name="cache">True to cache the offset, cache will be invalidated on DLL unload/load. If set to false, the value is NEVER cached.</param>
	signature(const std::string& signature_name, const std::wstring& module_name,
		std::initializer_list<int16_t> scan_bytes, int value_offset = 0, bool cache = true)
		: mSignatureName(signature_name), mModuleName(module_name), mScanBytes(scan_bytes),
		mValueOffset(value_offset), mCacheEnabled(cache), mCachedValue(nullptr)
	{
		assert(mScanBytes.size() > 0);
		assert(mSignatureName != "");
		assert(mModuleName != L"");
	}

	std::optional<T*> opt_value() const {
		if (mCacheEnabled && mCachedValue) {
			return mCachedValue;
		}

		auto dll = dll_cache::get_module_info(mModuleName);
		if (dll.has_value()) {
			const auto& module_info = dll.value();
			auto scanBytesSize = mScanBytes.size();

			int64_t startAddr = (int64_t)module_info.lpBaseOfDll;
			int64_t endAddr = startAddr + module_info.SizeOfImage + scanBytesSize;

			for (int64_t addr = startAddr; addr++; addr < endAddr) {

				uint8_t* ptr = reinterpret_cast<uint8_t*>(addr);
				for (int i = 0; i < scanBytesSize; i++) {
					if (mScanBytes[i] == -1) {
						continue;
					}
					if (ptr[i] != mScanBytes[i]) {
						break;
					}

					// Exit when we found the address matches
					if (i == scanBytesSize - 1) {
						ptr += mValueOffset;

						if (mCacheEnabled) {
							mCachedValue = (T*)ptr;
						}

						return (T*)ptr;
					}
				}
			}

		}

		return std::nullopt;
	}
};

template <typename T>
T* relative_offset_to_value(int64_t startAddr, int64_t relativeOffset, int instructionOffset) {
	auto addr = startAddr + relativeOffset + instructionOffset;
	return (T*)addr;
}

// Dont use these directly unless implementing accessors

const signature<int32_t> sig_halo1_cheat_table_relative("halo1_cheat_table_relative", HALO1_DLL_WSTR,
	//           relative offset
	{ 0x88, 0x05, -1, -1, -1, -1, 0x48, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0x84, 0xC0, 0x4C }, 2);

const signature<int32_t> sig_halo1_skull_table_offset("halo1_skull_table_offset", HALO1_DLL_WSTR,
	//                        absolute offset
	{ 0x0F, 0xb6, 0x84, 0x2b, -1, -1, -1, -1, 0x38, -1, -1, -1, -1, -1, -1, 0x74 }, 4);

const signature<int32_t> sig_halo2_skull_table_offset("halo2_skull_table_offset", HALO2_DLL_WSTR,
	//                          absolute offset
	{ 0x0f, 0xb6, 0x84, 0x2b, -1, -1, -1, -1, 0x38, -1, -1, -1, -1, -1, -1, 0x74 }, 4);

#include "pch.h"
#include "dll_cache.h"

void dll_cache::initialize()
{
	auto& instance = get();

	// Clear the cache
	instance.mCache.clear();

	// Fill with current values
	HMODULE hMods[1024];
	HANDLE hProcess = GetCurrentProcess();
	DWORD cbNeeded;

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];

			if (GetModuleBaseName(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				MODULEINFO info;
				if (GetModuleInformation(hProcess, hMods[i], &info, sizeof(info))) {
					std::wstring name{ szModName };
					instance.mCache.emplace(name, (HMODULE)info.lpBaseOfDll);
				}
			}
		}
	}

	tas_logger::trace("dll_cache initialized with {} DLLs.", instance.mCache.size());
}

bool dll_cache::add_to_cache(const std::wstring& dllName, HMODULE handle)
{
	auto& instance = get();

	// check if in cache
	auto it = instance.mCache.find(dllName);
	if (it != instance.mCache.end()) {
		return false;
	}

	// if not add to cache
	instance.mCache.emplace(dllName, handle);
	return true;
}

bool dll_cache::remove_from_cache(const std::wstring& dllName)
{
	auto& instance = get();
	auto it = instance.mCache.find(dllName);
	if (it != instance.mCache.end()) {
		instance.mCache.erase(it);
		return true;
	}

	return false;
}

std::optional<HMODULE> dll_cache::get_info(const std::wstring& dllName)
{
	auto& instance = get();
	auto it = instance.mCache.find(dllName);
	if (it != instance.mCache.end()) {
		return it->second;
	}

	return std::nullopt;
}

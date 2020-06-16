#include "pch.h"
#include "windows_utilities.h"

void fill_loaded_dlls_info(std::vector<loaded_dll_info>& dlls) {
    HMODULE hMods[1024];
    HANDLE hProcess = GetCurrentProcess();
    DWORD cbNeeded;

    // TODO-SCALES: This is quite slow, taking around 10 milliseconds to enumerate all DLLs.
    // Caching these and checking less often may be preferable.
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];

            if (GetModuleBaseName(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
                for (auto& dll : dlls) {
                    if (!wcscmp(szModName, dll.name.c_str())) {
                        GetModuleInformation(hProcess, hMods[i], &dll.info, sizeof(dll.info));
                    }
                }
            }
        }
    }
}

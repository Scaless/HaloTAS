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

void patch_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size)
{
    unsigned long old_protection, unused;
    //give that address read and write permissions and store the old permissions at oldProtection
    VirtualProtect(dest_address, patch_size, PAGE_EXECUTE_READWRITE, &old_protection);

    //write the memory into the program and overwrite previous value
    std::copy_n(src_address, patch_size, static_cast<uint8_t*>(dest_address));

    //reset the permissions of the address back to oldProtection after writting memory
    VirtualProtect(dest_address, patch_size, old_protection, &unused);
}
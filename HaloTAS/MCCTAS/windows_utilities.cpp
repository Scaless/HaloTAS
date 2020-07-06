#include "windows_utilities.h"

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

std::wstring str_to_wstr(const std::string str)
{
    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wStr = new wchar_t[wchars_num];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wStr, wchars_num);
    return std::wstring(wStr);
}

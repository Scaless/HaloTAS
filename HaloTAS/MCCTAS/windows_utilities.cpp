#include "pch.h"
#include "windows_utilities.h"

void make_minidump(EXCEPTION_POINTERS* e)
{
	auto hDbgHelp = LoadLibraryA("dbghelp");
	if (hDbgHelp == nullptr)
		return;
	auto pMiniDumpWriteDump = (decltype(&MiniDumpWriteDump))GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
	if (pMiniDumpWriteDump == nullptr)
		return;

	char name[MAX_PATH];
	{
		auto nameEnd = name + GetModuleFileNameA(GetModuleHandleA(0), name, MAX_PATH);
		SYSTEMTIME t;
		GetSystemTime(&t);
		wsprintfA(nameEnd - strlen("MCC-Win64-Shipping.exe"),
			"..\\..\\..\\MCCTAS\\MCCDMP_%4d%02d%02d_%02d%02d%02d.dmp",
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
	}

	auto hFile = CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = e;
	exceptionInfo.ClientPointers = FALSE;

	pMiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
		e ? &exceptionInfo : nullptr,
		nullptr,
		nullptr);

	CloseHandle(hFile);

	return;
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

std::wstring str_to_wstr(const std::string str)
{
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wStr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wStr, wchars_num);
	return std::wstring(wStr);
}

static LPTOP_LEVEL_EXCEPTION_FILTER OriginalUnhandledExceptionFilter;
LONG CALLBACK unhandled_handler(EXCEPTION_POINTERS* e)
{
	tas_logger::critical("Unhandled exception, creating minidump!");
	make_minidump(e);
	return EXCEPTION_CONTINUE_SEARCH;
}

void acquire_global_unhandled_exception_handler()
{
	OriginalUnhandledExceptionFilter = SetUnhandledExceptionFilter(unhandled_handler);
}

void release_global_unhandled_exception_handler()
{
	if (OriginalUnhandledExceptionFilter) {
		SetUnhandledExceptionFilter(OriginalUnhandledExceptionFilter);
	}
}

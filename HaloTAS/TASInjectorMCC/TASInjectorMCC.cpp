// TASInjectorMCC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

wchar_t HaloProcess[] = L"MCC-Win64-Shipping.exe";
std::string dllpath;
typedef HINSTANCE(*fpLoadLibrary)(char*);

bool InjectDLL(DWORD ProcessId) {
	HANDLE hProc;
	LPVOID paramAddr;

	HINSTANCE hDll = LoadLibrary(L"KERNEL32");
	fpLoadLibrary LoadLibraryAddr = (fpLoadLibrary)GetProcAddress(hDll, "LoadLibraryA");

	hProc = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessId);

	char dllPathChar[250];
	strcpy_s(dllPathChar, dllpath.c_str());

	paramAddr = VirtualAllocEx(hProc, 0, strlen(dllPathChar) + 1, MEM_COMMIT, PAGE_READWRITE);
	bool memoryWritten = WriteProcessMemory(hProc, paramAddr, dllPathChar, strlen(dllPathChar) + 1, NULL);

	CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, paramAddr, 0, 0);

	CloseHandle(hProc);

	return memoryWritten;
}

int main()
{
	std::string working_dir = fs::current_path().string();
	dllpath = working_dir + "/MCCTAS.dll";

	// Verify MCCTAS.dll exists
	if (!fs::exists(dllpath)) {
		std::cout << "DLL does not exist at path: " << dllpath << std::endl << "Press Enter to Close.";
		std::cin.get();
		return 0;
	}
	std::cout << "Found DLL at: " << dllpath << std::endl;

	DWORD procId = NULL;
	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
	HANDLE hProcSnap;

	while (!procId) {
		std::wcout << "Searching for " << HaloProcess << "..." << std::endl;
		hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (Process32First(hProcSnap, &pe32)) {
			do {
				if (!wcscmp(pe32.szExeFile, HaloProcess)) {
					procId = pe32.th32ProcessID;
					break;
				}
			} while (Process32Next(hProcSnap, &pe32));
		}
		Sleep(1000);

		CloseHandle(hProcSnap);
	}

	while (!InjectDLL(procId)) {
		std::cout << "DLL failed to inject." << std::endl << "Press Enter to Retry.";
		std::cin.get();
	}

	std::cout << "DLL injected." << std::endl;
	Sleep(1000);
}

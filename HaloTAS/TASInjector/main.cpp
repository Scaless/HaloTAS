#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <filesystem>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;

#define HALO_VANILLA
//#define HALO_CUSTOMED

#if defined(HALO_VANILLA) && defined(HALO_CUSTOMED)
#error "Don't define HALO_VANILLA and HALO_CUSTOMED at the same time."
#endif

#if !defined(HALO_VANILLA) && !defined(HALO_CUSTOMED)
#error "Either HALO_VANILLA or HALO_CUSTOMED has to be defined."
#endif

#if defined(HALO_VANILLA)
char HaloProcess[] = "halo.exe";
#elif defined(HALO_CUSTOMED)
char HaloProcess[] = "haloce.exe";
#endif

std::string dllpath;

typedef HINSTANCE (*fpLoadLibrary)(char*);

bool InjectDLL(DWORD ProcessId) {
	HANDLE hProc;
	LPVOID paramAddr;

	HINSTANCE hDll = LoadLibrary("KERNEL32");
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

int main() {

	std::string working_dir = std::experimental::filesystem::current_path().string();
	std::cout << "Checking for config in: " << working_dir << std::endl;
	ptree conf;

	std::string confPath = working_dir + "/config.json";

	if (std::experimental::filesystem::exists(confPath)) {
		read_json("config.json", conf);
		dllpath = conf.get<std::string>("dllpath") + "/TASDLL.dll";
	}
	else
	{
		std::cout << "No config.json found, trying current working dir..." << std::endl;
		dllpath = working_dir + "/TASDLL.dll";
	}

	if (!std::experimental::filesystem::exists(dllpath)) {
		std::cout << "DLL does not exist at path: " << dllpath << std::endl << "Press Enter to Close.";
		std::cin.get();
		return 0;
	}

	std::cout << "Found DLL at: " << dllpath << std::endl;
	
	DWORD procId = NULL;
	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
	HANDLE hProcSnap;

	while (!procId) {
		std::cout << "Searching for " << HaloProcess << "..." << std::endl;
		hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (Process32First(hProcSnap, &pe32)) {
			do {
				if (!strcmp(pe32.szExeFile, HaloProcess)) {
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

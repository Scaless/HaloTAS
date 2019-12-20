#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <filesystem>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;
namespace fs = std::filesystem;

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
	std::string working_dir = fs::current_path().string();
	std::cout << "Checking for config in: " << working_dir << std::endl;
	ptree conf;

	std::string confPath = working_dir + "/config.json";
	std::string halo_dir;

	// Get Config
	if (fs::exists(confPath)) {
		read_json(confPath, conf);
		auto dllpathVal = conf.get_optional<std::string>("dllpath");
		if (dllpathVal.has_value()) {
			dllpath = dllpathVal.value() + "/TASDLL.dll";
		}
		else {
			dllpath = working_dir + "/TASDLL.dll";
		}

		auto halodirVal = conf.get_optional<std::string>("halo_dir");
		if (halodirVal.has_value()) {
			halo_dir = halodirVal.value();
		}
		else {
			halo_dir = "C:/Program Files(x86)/Microsoft Games/Halo";
		}
	}
	else
	{
		std::cout << "No config.json found, using defaults..." << std::endl;
		dllpath = working_dir + "/TASDLL.dll";
		halo_dir = "C:/Program Files(x86)/Microsoft Games/Halo";
	}

	// Verify TASDLL.dll exists
	if (!fs::exists(dllpath)) {
		std::cout << "DLL does not exist at path: " << dllpath << std::endl << "Press Enter to Close.";
		std::cin.get();
		return 0;
	}
	std::cout << "Found DLL at: " << dllpath << std::endl;

	// Copy resources files to Halo directory
	/*if (fs::exists(halo_dir + "/" + HaloProcess)) {
		fs::copy("Resources", halo_dir, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
		std::cout << "Copied resources to " << halo_dir << std::endl;
	}
	else {
		std::cout << "Couldn't find halo.exe at path: " << halo_dir << std::endl
			<< "Check your config.json" << std::endl
			<< "Press Enter to Close.";
		std::cin.get();
		return 0;
	}*/
	
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

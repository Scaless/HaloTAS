#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <filesystem>

char HaloProcess[] = "haloce.exe";
char dllPath[250] = "C:\\Repos\\HaloTAS\\HaloTAS\\Release\\TASDLL.dll";

typedef HINSTANCE (*fpLoadLibrary)(char*);

bool InjectDLL(DWORD ProcessId) {
	HANDLE hProc;
	LPVOID paramAddr;

	HINSTANCE hDll = LoadLibrary("KERNEL32");
	fpLoadLibrary LoadLibraryAddr = (fpLoadLibrary)GetProcAddress(hDll, "LoadLibraryA");

	hProc = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessId);

	paramAddr = VirtualAllocEx(hProc, 0, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	bool memoryWritten = WriteProcessMemory(hProc, paramAddr, dllPath, strlen(dllPath) + 1, NULL);

	CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, paramAddr, 0, 0);

	CloseHandle(hProc);

	return memoryWritten;
}

int main() {
	if (!std::experimental::filesystem::exists(dllPath)) {
		printf("DLL does not exist at path: %s\n", dllPath);
		Sleep(1000);
		return 0;
	}
	
	DWORD procId = NULL;
	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
	HANDLE hProcSnap;

	while (!procId) {
		printf("Searching for %s...\n", HaloProcess);
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
		printf("DLL failed to inject.\n");
		Sleep(1000);
	}

	printf("DLL injected.\n");
	Sleep(1000);
}

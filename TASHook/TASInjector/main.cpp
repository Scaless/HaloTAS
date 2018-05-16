#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

using namespace std;

char HaloProcess[] = "halo.exe";
char dllPath[250] = "C:\\Repos\\HaloTAS\\TASHook\\Release\\TASDLL.dll";

typedef HINSTANCE (*fpLoadLibrary)(char*);

HANDLE threadHandle;

bool InjectDLL(DWORD ProcessId) {
	HANDLE hProc;
	LPVOID paramAddr;

	HINSTANCE hDll = LoadLibrary("KERNEL32");

	fpLoadLibrary LoadLibraryAddr = (fpLoadLibrary)GetProcAddress(hDll, "LoadLibraryA");

	hProc = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessId);

	paramAddr = VirtualAllocEx(hProc, 0, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	bool memoryWritten = WriteProcessMemory(hProc, paramAddr, dllPath, strlen(dllPath) + 1, NULL);

	threadHandle = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, paramAddr, 0, 0);

	CloseHandle(hProc);

	return memoryWritten;
}

int main() {

	DWORD procId = NULL;
	PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };
	HANDLE hProcSnap;

	while (!procId) {
		cout << "Searching for " << HaloProcess << "..." << endl;
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
	}

	while (!InjectDLL(procId)) {
		cout << "DLL failed to inject" << endl;
		Sleep(1000);
	}

	cout << "DLL injected" << endl;

	CloseHandle(hProcSnap);
/*
	while (true) {
		cout << "Press Enter to unload dll." << endl;
		getchar();
		CloseHandle(threadHandle);
	}*/

	Sleep(1000);

	return 0;
}


#include <Windows.h>
#include <stdio.h>

int UIMain() {
	FILE *file;
	fopen_s(&file, "C:\\HaloTAS\\temp.txt", "a+");
	fprintf(file, "DLL attach function called.\n");
	fclose(file);

	return S_OK;
}

DWORD WINAPI Main_Thread(LPVOID lpParam) {
	UIMain();
	return S_OK;
}

BOOL APIENTRY DLLMain(HMODULE hModule, DWORD _reason, LPVOID lpReserved) {
	if (_reason == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0x1000, &Main_Thread, 0, 0, NULL);
	}

	return TRUE;
}
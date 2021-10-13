#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <filesystem>
#include <thread>
#include <vector>
#include <algorithm>
#include "../libhalotas/hook.h"
#include "../libhalotas/dll_cache.h"

typedef HMODULE(*LoadLibraryA_t)(LPCSTR lpLibFileName);
HMODULE hkLoadLibraryA(LPCSTR lpLibFileName);
LoadLibraryA_t originalLoadLibraryA;

typedef HMODULE(*LoadLibraryW_t)(LPCWSTR lpLibFileName);
HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName);
LoadLibraryW_t originalLoadLibraryW;

typedef HMODULE(*LoadLibraryExA_t)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
LoadLibraryExA_t originalLoadLibraryExA;

typedef HMODULE(*LoadLibraryExW_t)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
LoadLibraryExW_t originalLoadLibraryExW;

typedef BOOL(*FreeLibrary_t)(HMODULE hLibModule);
BOOL hkFreeLibrary(HMODULE hLibModule);
FreeLibrary_t originalFreeLibrary;

// H1 Carrier Crash
typedef uint64_t(__fastcall* CarrierFreezeOuter)(int64_t p1, char p2, uint64_t p3, float p4, uint64_t p5, float p6, float* p7);
uint64_t __fastcall hkCarrierFreezeOuter(int64_t p1, char p2, uint64_t p3, float p4, uint64_t p5, float p6, float* p7);
CarrierFreezeOuter originalCarrierFreezeOuter;

typedef bool(__fastcall* CarrierFreezeInner)(int64_t p1, int32_t p2, uint64_t p3, uint64_t p4, float* p5, float* p6, uint64_t* p7);
bool __fastcall hkCarrierFreezeInner(int64_t p1, int32_t p2, uint64_t p3, uint64_t p4, float* p5, float* p6, uint64_t* p7);
CarrierFreezeInner originalCarrierFreezeInner;

// H2 BSP Crash
//typedef void*(*BSPClearPointerTable)();
//void* hkBSPClearPointerTable();
//BSPClearPointerTable originalBSPClearPointerTable;
//
//typedef int64_t(*BSPGetPointer)(int64_t index);
//int64_t hkBSPGetPointer(int64_t index);
//BSPGetPointer originalBSPGetPointer;
//
//typedef int64_t(*BSPAddPointer)(int64_t new_param);
//int64_t hkBSPAddPointer(int64_t new_param);
//BSPAddPointer originalBSPAddPointer;
//
//void CopyExistingPointerTable();

// Hooks
std::vector<hook> gGlobalHooks;
std::vector<hook> gRuntimeHooks;

void attach_global_hooks() {
	for (auto& hk : gGlobalHooks) {
		hk.attach();
	}
}

void attach_runtime_hooks() {
	for (auto& hook : gRuntimeHooks) {
		hook.attach();
	}
}

void init_global_hooks() {
	originalLoadLibraryA = LoadLibraryA;
	originalLoadLibraryW = LoadLibraryW;
	originalLoadLibraryExA = LoadLibraryExA;
	originalLoadLibraryExW = LoadLibraryExW;
	originalFreeLibrary = FreeLibrary;
}

std::wstring str_to_wstr(const std::string str)
{
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wStr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wStr, wchars_num);
	return std::wstring(wStr);
}

void post_lib_load_hooks_patches(std::wstring_view libPath) {
	std::filesystem::path path = libPath;
	auto filename = path.filename().generic_wstring();

	for (auto& hook : gRuntimeHooks) {
		if (hook.module_name() == filename) {
			hook.attach();
		}
	}
}

void pre_lib_unload_hooks_patches(std::wstring_view libFilename) {
	for (auto& hook : gRuntimeHooks) {
		if (hook.module_name() == libFilename) {
			hook.detach();
		}
	}
}

HMODULE hkLoadLibraryA(LPCSTR lpLibFileName) {
	auto result = originalLoadLibraryA(lpLibFileName);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	dll_cache::add_to_cache(wLibFileName, result);
	post_lib_load_hooks_patches(wLibFileName);

	return result;
}

HMODULE hkLoadLibraryW(LPCWSTR lpLibFileName) {
	auto result = originalLoadLibraryW(lpLibFileName);

	dll_cache::add_to_cache(lpLibFileName, result);
	post_lib_load_hooks_patches(lpLibFileName);

	return result;
}

HMODULE hkLoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = originalLoadLibraryExA(lpLibFileName, hFile, dwFlags);

	auto wLibFileName = str_to_wstr(lpLibFileName);
	dll_cache::add_to_cache(wLibFileName, result);
	post_lib_load_hooks_patches(wLibFileName);

	return result;
}

HMODULE hkLoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
	auto result = originalLoadLibraryExW(lpLibFileName, hFile, dwFlags);

	dll_cache::add_to_cache(lpLibFileName, result);
	post_lib_load_hooks_patches(lpLibFileName);

	return result;
}

BOOL hkFreeLibrary(HMODULE hLibModule) {
	wchar_t moduleFilePath[MAX_PATH];
	GetModuleFileName(hLibModule, moduleFilePath, sizeof(moduleFilePath) / sizeof(TCHAR));

	std::filesystem::path path = moduleFilePath;
	auto filename = path.filename().generic_wstring();

	pre_lib_unload_hooks_patches(filename);
	dll_cache::remove_from_cache(filename);

	return originalFreeLibrary(hLibModule);
}

void PatcherMain()
{
	dll_cache::initialize();

	gGlobalHooks.push_back(hook(L"hkLoadLibraryA", (PVOID**)&originalLoadLibraryA, hkLoadLibraryA));
	gGlobalHooks.push_back(hook(L"hkLoadLibraryW", (PVOID**)&originalLoadLibraryW, hkLoadLibraryW));
	gGlobalHooks.push_back(hook(L"hkLoadLibraryExA", (PVOID**)&originalLoadLibraryExA, hkLoadLibraryExA));
	gGlobalHooks.push_back(hook(L"hkLoadLibraryExW", (PVOID**)&originalLoadLibraryExW, hkLoadLibraryExW));
	gGlobalHooks.push_back(hook(L"hkFreeLibrary", (PVOID**)&originalFreeLibrary, hkFreeLibrary));

	init_global_hooks();
	attach_global_hooks();

	/// HALO 1

	///////////////////////////////
	// 	   Carrier Freeze        //
	///////////////////////////////
	// 2094: 0xd4c1d0
	// 2241: 0xd47650
	// 2282: 0xd47680
	// 2406: 0xd40fc0
	// 2448: 0xd40fc0
	// 2580: 0xd46d00
	gRuntimeHooks.push_back(hook(L"CarrierFreezeOuter", L"halo1.dll", 0xd46d00, (PVOID**)&originalCarrierFreezeOuter, hkCarrierFreezeOuter));
	// 2094: 0xc8a470
	// 2241: 0xc90ca0
	// 2282: 0xc90cd0
	// 2406: 0xc878c0
	// 2448: 0xc878c0
	// 2580: 0xc93bb0
	gRuntimeHooks.push_back(hook(L"CarrierFreezeInner", L"halo1.dll", 0xc93bb0, (PVOID**)&originalCarrierFreezeInner, hkCarrierFreezeInner));
	///////////////////////////////

	/// HALO 2
	
	///////////////////////////////
	// 	   BSP Crash             //
	///////////////////////////////
	// 2282: 0x6df770
	// 2406: 0x6df710
	// 2448: 0x6df710
	// 2580: FIXED? 
	//gRuntimeHooks.push_back(hook(L"hkBSPClearPointerTable", L"halo2.dll", 0x6df710, (PVOID**)&originalBSPClearPointerTable, hkBSPClearPointerTable));
	// 2282: 0x6df7a0
	// 2406: 0x6df740
	// 2448: 0x6df740
	// 2580: FIXED? 
	//gRuntimeHooks.push_back(hook(L"hkBSPGetPointer", L"halo2.dll", 0x6df740, (PVOID**)&originalBSPGetPointer, hkBSPGetPointer));
	// 2282: 0x6df7b0
	// 2406: 0x6df750
	// 2448: 0x6df750
	// 2580: FIXED? 
	//gRuntimeHooks.push_back(hook(L"hkBSPAddPointer", L"halo2.dll", 0x6df750, (PVOID**)&originalBSPAddPointer, hkBSPAddPointer));
	// Copy the existing pointer table to our bigger buffer
	//CopyExistingPointerTable();
	///////////////////////////////

	attach_runtime_hooks();

	while (true)
	{
		Sleep(100);
	}
}

// This thread is created by the dll when loaded into the process, see PatcherMain() for the actual event loop.
// Do NOT put any allocations in this function because the call to FreeLibraryAndExitThread()
// will occur before they fall out of scope and will not be cleaned up properly! This is very
// important for being able to hotload the DLL multiple times without restarting the game.
DWORD WINAPI MainThread(HMODULE hDLL) {
	PatcherMain();

	Sleep(200);
	FreeLibraryAndExitThread(hDLL, NULL);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		DWORD dwThreadID;
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, &dwThreadID);
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

/// <summary>
/// Halo 1 Carrier Crashes
/// </summary>
/// 
// This seems to be a timing issue that is causing the infinite loop / freeze.
// The solution I have here is to keep a counter of loop iterations that will reset when we successfully complete the outer function.
// If we reach an excessive amount of iterations (10k) in the inner loop, stall the thread so that whatever mechanism is out of sync has time to do its thing.
// This should not affect any normal code paths, the only time this should trigger is during an abnormal freeze.
// Compiler optimizations with these hooked functions cause a crash on release mode, not sure why. 
#pragma optimize("", off)
int64_t FreezeCounter = 0;
uint64_t __fastcall hkCarrierFreezeOuter(int64_t p1, char p2, uint64_t p3, float p4, uint64_t p5, float p6, float* p7)
{
	FreezeCounter = 0;
	return originalCarrierFreezeOuter(p1, p2, p3, p4, p5, p6, p7);
}
bool __fastcall hkCarrierFreezeInner(int64_t p1, int32_t p2, uint64_t p3, uint64_t p4, float* p5, float* p6, uint64_t* p7)
{
	FreezeCounter++;
	if (FreezeCounter > 10'000) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	return originalCarrierFreezeInner(p1, p2, p3, p4, p5, p6, p7);
}
#pragma optimize("", on)

/// <summary>
/// Halo 2 "BSP Crashes"
/// </summary>
/// 
/// The basic problem causing this crash is a buffer overrun. There exists a table of tag pointers that 
/// has an increasing counter (Tag_Pointer_Table_Current_Index). This index is reset only once when the level is first loaded,
/// after that point it increases with no upper bound until eventually overrunning and corrupting memory it does not control.
/// 
/// The memory layout looks something like this:
/// [Tag Pointer Table][BSP Debug Flag][Scenario Header] ... [Tag_Pointer_Table_Current_Index]
/// 
/// Despite generally being called a "BSP Crash", the cause has nothing to do with BSPs. The tag pointer table simply
/// overruns and incidentally sets the flag that causes the BSP debug string to show on screen. Immediately after that in 
/// memory is the scenario data which is critical for running the level. Once scenario data is corrupted, the crash occurs.
/// 
/// The solution given below is to redirect the tag pointers into a new table that has expandable storage.

//std::vector<int64_t> PointerTable;
//
//int32_t* GetNativePointerIndex()
//{
//	auto dll = dll_cache::get_module_handle(L"halo2.dll");
//	if (dll.has_value()) {
//		char* module_ptr = (char*)dll.value();
//		// 2282: 0xcd8098
//		// 2406: 0xcd9098
//		// 2448: 0xcd9098
//		// 2580: FIXED?
//		return (int32_t*)(module_ptr + 0xcd9098);
//	}
//	else {
//		// If we somehow call into this function when we don't have a value, we're fucked anyways so just return a nullptr to crash out
//		return nullptr;
//	}
//}
//
//void CopyExistingPointerTable()
//{
//	auto dll = dll_cache::get_module_handle(L"halo2.dll");
//	if (dll.has_value())
//	{
//		PointerTable.resize(*GetNativePointerIndex());
//		// 2282: 0xe22370
//		// 2406: 0xe23110
//		// 2448: 0xe23110
//		// 2580: FIXED?
//		uint64_t* existing_table_start = (uint64_t*)(((char*)dll.value()) + 0xe23110);
//		memcpy_s(PointerTable.data(), PointerTable.size() * sizeof(uint64_t), existing_table_start, *GetNativePointerIndex() * sizeof(uint64_t));
//	}
//}
//
//#pragma optimize("", off)
//void* hkBSPClearPointerTable()
//{
//	PointerTable.clear();
//	PointerTable.push_back(0);
//	*GetNativePointerIndex() = 1;
//	return PointerTable.data();
//}
//
//int64_t hkBSPGetPointer(int64_t index)
//{
//	return PointerTable[index];
//}
//
//int64_t hkBSPAddPointer(int64_t new_param)
//{
//	int64_t current_index = *GetNativePointerIndex();
//	PointerTable.push_back(new_param);
//	(*GetNativePointerIndex())++;
//	return current_index;
//}
//#pragma optimize("", on)

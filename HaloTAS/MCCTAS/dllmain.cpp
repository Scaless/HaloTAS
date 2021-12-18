// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "global.h"
#include "windows_utilities.h"
#include "windows_console.h"
#include "tas_hooks.h"
#include "gui_interop.h"
#include "dll_cache.h"
#include "signatures.h"
#include <TlHelp32.h>
#include <winternl.h>

BOOL ListProcessThreads(DWORD dwOwnerPID)
{
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;

	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32);

	// Retrieve information about the first thread,
	// and exit if unsuccessful
	if (!Thread32First(hThreadSnap, &te32))
	{
		CloseHandle(hThreadSnap);     // Must clean up the snapshot object!
		return(FALSE);
	}

	HMODULE mod = GetModuleHandle(L"HaloInfinite.exe");
	char* exe_base_addr = (char*)mod;

	// Now walk the thread list of the system,
	// and display information about each thread
	// associated with the specified process
	do
	{
		if (te32.th32OwnerProcessID == dwOwnerPID)
		{
			typedef struct _THREAD_BASIC_INFORMATION
			{
				NTSTATUS ExitStatus;
				PTEB TebBaseAddress;
				CLIENT_ID ClientId;
				ULONG_PTR AffinityMask;
				KPRIORITY Priority;
				LONG BasePriority;
			} THREAD_BASIC_INFORMATION, * PTHREAD_BASIC_INFORMATION;

			THREAD_BASIC_INFORMATION bi;
			NT_TIB tib;

			void* Tinfo = 0;
			ULONG retlen = 0;
			{
				HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
				NtQueryInformationThread(hThread, (THREADINFOCLASS)9, &Tinfo, sizeof(PVOID), &retlen);
				CloseHandle(hThread);
			}
			{
				HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
				NtQueryInformationThread(hThread, (THREADINFOCLASS)0, &bi, sizeof(THREAD_BASIC_INFORMATION), NULL);
				CloseHandle(hThread);
			}
			
			CONTEXT threadContext = {0};
			threadContext.ContextFlags = CONTEXT_SEGMENTS;
			{
				HANDLE hThread = OpenThread(THREAD_GET_CONTEXT, FALSE, te32.th32ThreadID);
				GetThreadContext(hThread, &threadContext);
				CloseHandle(hThread);
			}

			if (Tinfo == (exe_base_addr + 0x467010))
			{
				tas_logger::warning(TEXT("Main Thread ID = {}\t | Start Addr = {}"), te32.th32ThreadID, fmt::ptr(Tinfo));

				ReadProcessMemory(GetCurrentProcess(), bi.TebBaseAddress, &tib, sizeof(NT_TIB), 0);
				
				uint32_t TLSIndex = *(uint32_t*)(exe_base_addr + 0x49DF8BC);

				// Determine if we are using the base 64 slots or the 1024 overflow slots
				char* checkpoint_addr;
				if (TLSIndex < 64)
				{
					uint64_t* slot = (uint64_t*)bi.TebBaseAddress->TlsSlots;
					checkpoint_addr = (char*)slot[TLSIndex];
				}
				else
				{
					uint64_t* slot = (uint64_t*)bi.TebBaseAddress->TlsExpansionSlots;
					checkpoint_addr = (char*)slot[TLSIndex - 64];
				}

				tas_logger::error(L"Checkpoint Addr: {}", fmt::ptr(checkpoint_addr));

				for (int i = 0; i < 64; i++)
				{
					uint64_t* slot = (uint64_t*)bi.TebBaseAddress->TlsSlots;
					char* addr = (char*)slot[i];
					if (addr != nullptr)
					{
						tas_logger::error(L"Addr: {}", fmt::ptr(addr));
					}
				}
				for (int i = 0; i < 1024; i++)
				{
					uint64_t* slot = (uint64_t*)bi.TebBaseAddress->TlsExpansionSlots;
					char* addr = (char*)slot[i];
					if (addr != nullptr)
					{
						tas_logger::error(L"Addr: {}", fmt::ptr(addr));
					}
				}

				*checkpoint_addr = 1;
				tas_logger::warning(L"Set checkpoint flag!");

			}
		}
	} while (Thread32Next(hThreadSnap, &te32));


	//  Don't forget to clean up the snapshot object.
	CloseHandle(hThreadSnap);
	return(TRUE);
}

void DumpHeaps()
{
	constexpr size_t runtime_data_size = 0x4B30000;

	char* address{ 0 };
	MEMORY_BASIC_INFORMATION region;

	char* reg = nullptr;

	while (VirtualQuery(address, &region, sizeof(region)))
	{
		address += region.RegionSize;
		if (region.RegionSize == 0x4B30000)
		{
			tas_logger::warning(L"Found Infinite Runtime Heap: {} - {}", fmt::ptr(region.BaseAddress), fmt::ptr(((char*)region.BaseAddress) + region.RegionSize));
			reg = (char*)region.BaseAddress;
		}
	}

	ListProcessThreads(GetCurrentProcessId());

	/*std::vector<char> bytes;
	bytes.resize(runtime_data_size);
	
	memcpy(bytes.data(), reg, runtime_data_size);

	tas_logger::warning(L"Copied Runtime Heap: {}", fmt::ptr(reg));

	using namespace std::chrono_literals;
	std::this_thread::sleep_for(10000ms);

	tas_logger::warning(L"Injecting into Runtime Heap: {}", fmt::ptr(reg));*/

	//memcpy(reg, bytes.data(), runtime_data_size);

}

// Main Execution Loop
void RealMain() {
	acquire_global_unhandled_exception_handler();

	auto consoleWindow = std::make_unique<windows_console>();
	consoleWindow->open();

	const std::string startupMessage =
		"MCCTAS Started!\r\n"
		"To close MCCTAS, press CTRL + C while this window is focused. MCC will continue running.\r\n"
		"If you accidentally highlight something in this window, focus this window and press ESC to unfreeze the game.\r\n"
		"Press the tilde(~) key in-game to open the MCCTAS console. Enter the 'help' command for more information on using the console.\r\n\r\n"
		"MCCTAS significantly modifies game behavior and is not approved for official speedruns.\r\n"
		"Report bugs and suggest features on Github at: https://github.com/Scaless/HaloTAS. \r\n"
		"Have fun!\r\n~Scales\r\n";

	tas_logger::warning("{}", startupMessage);
	//tas_logger::warning("KNOWN ISSUES: \r\n\t Changing graphics options crashes the game.");

	DumpHeaps();

	dll_cache::initialize();

	auto interop = std::make_unique<gui_interop>();
	auto hooks = std::make_unique<tas_hooks>();
	hooks->attach_all();

	// Most of the cool stuff happens in other threads.
	// This loop is just to keep stuff alive.
	while (!global::is_kill_set()) {
		Sleep(100);
	}

	hooks->detach_all();

	tas_logger::info("MCCTAS Stopped!");
	tas_logger::flush_and_exit();
	release_global_unhandled_exception_handler();
}

// This thread is created by the dll when loaded into the process, see RealMain() for the actual event loop.
// Do NOT put any allocations in this function because the call to FreeLibraryAndExitThread()
// will occur before they fall out of scope and will not be cleaned up properly! This is very
// important for being able to hotload the DLL multiple times without restarting the game.
DWORD WINAPI MainThread(HMODULE hDLL) {
	RealMain();

	Sleep(200);
	FreeLibraryAndExitThread(hDLL, NULL);
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	DWORD dwThreadID;

	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, &dwThreadID);
	}

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

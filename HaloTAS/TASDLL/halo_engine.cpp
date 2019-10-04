#include "halo_engine.h"
#include <Windows.h>
#include <Psapi.h>

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
	char title[255];

	if (IsWindowVisible(hWnd)) {
		GetWindowText(hWnd, (LPSTR)title, 254);

		if (std::string(title) == "Halo") {
			DWORD processId;
			GetWindowThreadProcessId(hWnd, &processId);
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);

			char processName[255];
			GetProcessImageFileNameA(hProcess, processName, sizeof(processName));

			if (std::string(processName).find("halo.exe") != std::string::npos) {
				auto engine = reinterpret_cast<halo_engine*>(lParam);
				engine->set_window_handle(hWnd);
				return FALSE;
			}
		}
	}

	return TRUE;
}

void halo_engine::update_window_handle()
{
	EnumWindows(&EnumWindowsProc, reinterpret_cast<LPARAM>(this));
	//haloHWND = FindWindowA(nullptr, "Halo");
}

void halo_engine::patch_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size)
{
	unsigned long old_protection, unused;
	//give that address read and write permissions and store the old permissions at oldProtection
	VirtualProtect(dest_address, patch_size, PAGE_EXECUTE_READWRITE, &old_protection);

	//write the memory into the program and overwrite previous value
	std::copy_n(src_address, patch_size, static_cast<uint8_t*>(dest_address));

	//reset the permissions of the address back to oldProtection after writting memory
	VirtualProtect(dest_address, patch_size, old_protection, &unused);
}

void halo_engine::set_window_handle(HWND handle)
{
	haloHWND = handle;
}

halo_engine::halo_engine()
{
	// Scan memory for object pools
	uint32_t data_pool_size = 0x1B40000;

	for (uint32_t i = 0; i < data_pool_size / 4; i++) {
		if (memcmp(&MAGIC_DATAPOOLHEADER, &(ADDR_RUNTIME_DATA_BEGIN[i]), sizeof(MAGIC_DATAPOOLHEADER)) == 0) {
			DataPool* pool = (DataPool*)(&ADDR_RUNTIME_DATA_BEGIN[i] - 10);
			dataPools.push_back(pool);

			if (std::string(pool->Name) == "object") {
				objectDataPool = pool;
			}
		}
	}

	update_window_handle();
}

HWND halo_engine::window_handle()
{
	return haloHWND;
}

void halo_engine::get_snapshot(engine_snapshot& snapshot)
{
	if (objectDataPool != nullptr)
	{
		int count = objectDataPool->ObjectCount;
		for (int i = 0; i < count; i++) {
			uint32_t* basePtr = (uint32_t*)&objectDataPool->ObjectPointers[i];

			if ((int)*basePtr == 0) {
				continue;
			}
			if ((int)*basePtr < 0x40000000 || (int)*basePtr > 0x41B40000) {
				continue;
			}

			GameObject* object = (GameObject*)(*basePtr - 24); // Backtrack 24 bytes from the pointed to location
			snapshot.gameObjects.push_back(object);
		}
	}
}

void halo_engine::print_hud_text(const std::wstring& input)
{
	wchar_t copy[64];
	wcscpy_s(copy, 64, input.c_str());

	__asm {
		lea	eax, copy
		push eax
		mov	eax, 0 // player index
		call halo::function::PRINT_HUD
		add	esp, 4
	}
}

int halo_engine::get_tag_index_from_path(int tagIdentifier, char* path)
{
	int index = -1;
	__asm {
		mov edi, tagIdentifier
		push path
		call halo::function::GET_TAG_INDEX
		mov index, eax
		add esp, 4
	}
	return index;
}

void halo_engine::set_debug_camera(bool enabled)
{
	if (enabled) {
		ADDR_DEBUG_CAMERA_ENABLE[0] = 0x90;
		ADDR_DEBUG_CAMERA_ENABLE[1] = 0x6E;
	}
	else {
		ADDR_DEBUG_CAMERA_ENABLE[0] = 0x60;
		ADDR_DEBUG_CAMERA_ENABLE[1] = 0x6D;
	}
}

// Gets the name of a BSP from the level name and index
std::string halo_engine::current_bsp_name()
{
	std::string currentMap = std::string(ADDR_MAP_STRING);

	auto keyVal = LEVEL_BSP_NAME.find(std::make_pair(currentMap, *ADDR_CURRENT_BSP_INDEX));
	if (keyVal != LEVEL_BSP_NAME.end()) {
		return keyVal->second;
	}

	return std::string();
}

void halo_engine::mouse_directinput_override_disable()
{
	patch_memory(ADDR_PATCH_DINPUT_MOUSE, PATCH_DINPUT_MOUSE_ORIGINAL, 7);
}

void halo_engine::mouse_directinput_override_enable()
{
	patch_memory(ADDR_PATCH_DINPUT_MOUSE, PATCH_DINPUT_MOUSE_BYTES, 7);
}

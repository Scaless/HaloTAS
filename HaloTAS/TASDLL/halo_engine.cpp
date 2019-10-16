#include "halo_engine.h"
#include <Windows.h>
#include <Psapi.h>
#include "tas_input_handler.h"

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

void halo_engine::disable_render()
{
	*ADDR_ENGINE_RENDER_ENABLE = 1;
}

void halo_engine::enable_render()
{
	*ADDR_ENGINE_RENDER_ENABLE = 0;
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

	auto addr = &enableFastForward;
	patch_memory(ADDR_FAST_FORWARD_POINTER, (uint8_t*)& addr, 4);
}

halo_engine::~halo_engine()
{
	auto defaultAddr = 0x007196D8;
	patch_memory(ADDR_FAST_FORWARD_POINTER, (uint8_t*)& defaultAddr, 4);
}

HWND halo_engine::window_handle()
{
	return haloHWND;
}

RECT halo_engine::window_client_rect()
{
	if (haloHWND) {
		RECT outRect;
		if (GetClientRect(haloHWND, &outRect)) {
			return outRect;
		}
	}
	return RECT();
}

RECT halo_engine::window_window_rect()
{
	if (haloHWND) {
		RECT outRect;
		if (GetWindowRect(haloHWND, &outRect)) {
			return outRect;
		}
	}
	return RECT();
}

void halo_engine::focus()
{
	if (haloHWND) {
		SetForegroundWindow(haloHWND);
	}
}

void halo_engine::get_snapshot(engine_snapshot& snapshot)
{
	if (objectDataPool != nullptr)
	{
		int count = objectDataPool->ObjectCount;
		for (int i = 0; i < count; i++) {
			uint32_t* basePtr = (uint32_t*)& objectDataPool->ObjectPointers[i];

			if ((int)* basePtr == 0) {
				continue;
			}
			if ((int)* basePtr < 0x40000000 || (int)* basePtr > 0x41B40000) {
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

void halo_engine::fast_forward_to(uint32_t tick)
{
	if (tick <= 0) {
		fastForwardTick = 0;
		return;
	}

	if (!enableFastForward && fastForwardTick == 0) {
		fastForwardTick = tick;
		if (fastForwardTick < *ADDR_SIMULATION_TICK) {
			map_reset();
		}
	}
}

void halo_engine::map_reset()
{
	*ADDR_MAP_RESET = 1;
}

void halo_engine::core_save()
{
	*ADDR_CORE_SAVE = 1;
}

void halo_engine::core_load()
{
	*ADDR_CORE_LOAD = 1;
}

void halo_engine::save_checkpoint()
{
	*ADDR_SAVE_CHECKPOINT = 1;
}

void halo_engine::load_checkpoint()
{
	*ADDR_LOAD_CHECKPOINT = 1;
}

void halo_engine::pre_frame()
{
	if (fastForwardTick > 0 && fastForwardTick > * ADDR_SIMULATION_TICK) {
		enableFastForward = 1;
		if (fastForwardTick < *ADDR_SIMULATION_TICK + 45) {
			enable_render();
		}
		else {
			disable_render();
		}
	}
	else if (fastForwardTick > 0 && *ADDR_SIMULATION_TICK == fastForwardTick) {
		*ADDR_GAME_SPEED = 0;
		enableFastForward = 0;
		fastForwardTick = 0;
		enable_render();
	}
	else {
		enableFastForward = 0;
		fastForwardTick = 0;
		enable_render();
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

halo::MAP halo_engine::current_map()
{
	using halo::MAP;
	std::string currentMap = std::string(ADDR_MAP_STRING);

	if (currentMap == "levels\\a10\\a10") {
		return MAP::PILLAR_OF_AUTUMN;
	}
	else if (currentMap == "levels\\a30\\a30") {
		return MAP::HALO;
	}
	else if (currentMap == "levels\\a50\\a50") {
		return MAP::TRUTH_AND_RECONCILIATION;
	}
	else if (currentMap == "levels\\b30\\b30") {
		return MAP::SILENT_CARTOGRAPHER;
	}
	else if (currentMap == "levels\\b40\\b40") {
		return MAP::ASSAULT_ON_THE_CONTROL_ROOM;
	}
	else if (currentMap == "levels\\c10\\c10") {
		return MAP::_343_GUILTY_SPARK;
	}
	else if (currentMap == "levels\\c20\\c20") {
		return MAP::LIBRARY;
	}
	else if (currentMap == "levels\\c40\\c40") {
		return MAP::TWO_BETRAYALS;
	}
	else if (currentMap == "levels\\d20\\d20") {
		return MAP::KEYES;
	}
	else if (currentMap == "levels\\d40\\d40") {
		return MAP::MAW;
	}
	else if (currentMap == "levels\\ui\\ui") {
		return MAP::UI_MAIN_MENU;
	}
	else {
		return MAP::UNKNOWN_MAP;
	}
}

halo::DIFFICULTY halo_engine::current_difficulty()
{
	using halo::DIFFICULTY;
	switch (*ADDR_GAME_DIFFICULTY_ACTUAL) {
	case 0: return DIFFICULTY::EASY;
	case 1: return DIFFICULTY::NORMAL;
	case 2: return DIFFICULTY::HEROIC;
	case 3: return DIFFICULTY::LEGENDARY;
	default: return DIFFICULTY::INVALID_DIFFICULTY;
	}
}

void halo_engine::mouse_directinput_override_disable()
{
	patch_memory(ADDR_PATCH_DINPUT_MOUSE, PATCH_DINPUT_MOUSE_ORIGINAL, 7);
}

void halo_engine::mouse_directinput_override_enable()
{
	patch_memory(ADDR_PATCH_DINPUT_MOUSE, PATCH_DINPUT_MOUSE_BYTES, 7);
}

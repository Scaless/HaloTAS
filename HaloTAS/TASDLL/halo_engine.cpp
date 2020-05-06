#include "halo_engine.h"
#include <Windows.h>
#include <Psapi.h>
#include "tas_input_handler.h"
#include <fstream>

using namespace halo::addr;

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
	//*ENGINE_RENDER_ENABLE = 1;
}

void halo_engine::enable_render()
{
	//*ENGINE_RENDER_ENABLE = 0;
}

void halo_engine::disable_sound()
{
	//*SOUND_ENABLED = 0;
}

void halo_engine::enable_sound()
{
	//*SOUND_ENABLED = 1;
}

void halo_engine::set_window_handle(HWND handle)
{
	haloHWND = handle;
}

halo_engine::halo_engine()
{
	update_window_handle();
	disable_cutscene_fps_cap();
	disable_look_centering();

	auto addr = &enableFastForward;
	patch_memory(FAST_FORWARD_POINTER, (uint8_t*)& addr, 4);
}

halo_engine::~halo_engine()
{
	auto defaultAddr = 0x007196D8;
	patch_memory(FAST_FORWARD_POINTER, (uint8_t*)& defaultAddr, 4);

	enable_cutscene_fps_cap();
	enable_look_centering();
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

bool halo_engine::hac_is_loaded()
{
	HMODULE hMods[1024];
	HANDLE hProcess = GetCurrentProcess();
	DWORD cbNeeded;

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				std::string moduleNameStr{ szModName };
				auto found = moduleNameStr.find("hac.dll");

				if (found != std::string::npos) {
					return true;
				}
			}
		}
	}

	return false;
}

void halo_engine::get_snapshot(engine_snapshot& snapshot)
{
	object_pool_header* objectDataPool = halo::addr::OBJ_POOL_HEADER;
	if (objectDataPool != nullptr)
	{
		int count = objectDataPool->max_objects;
		for (int i = 0; i < count; i++) {
			object_pool_entry* object_entry = &objectDataPool->object_data_begin[i];

			char* basePtr = (char*)object_entry->object_address;

			if (basePtr != nullptr) {
				GameObject* object = (GameObject*)(basePtr - 24); // Backtrack 24 bytes from the pointed to location
				snapshot.gameObjects.push_back(object);
			}

		}
	}
}

bool halo_engine::is_present_enabled()
{
	return isPresentEnabled;
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
		DEBUG_CAMERA_ENABLE[0] = 0x90;
		DEBUG_CAMERA_ENABLE[1] = 0x6E;
	}
	else {
		DEBUG_CAMERA_ENABLE[0] = 0x60;
		DEBUG_CAMERA_ENABLE[1] = 0x6D;
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
		if (fastForwardTick < *SIMULATION_TICK) {
			map_reset();
		}
	}
}

void halo_engine::map_reset()
{
	*MAP_RESET = 1;
}

void halo_engine::core_save()
{
	*CORE_SAVE = 1;
}

void halo_engine::core_load()
{
	*CORE_LOAD = 1;
}

void halo_engine::core_save_full()
{
	queueFullCoreSave = true;
}

void halo_engine::core_load_full(int32_t tick_id)
{
	queueFullCoreLoad = true;
	queueFullCoreLoadTick = tick_id;
}

std::vector<uint32_t> halo_engine::get_cache_ticks()
{
	return core_cache.get_cached_ticks();
}

void halo_engine::save_checkpoint()
{
	*SAVE_CHECKPOINT = 1;
}

void halo_engine::load_checkpoint()
{
	*LOAD_CHECKPOINT = 1;
}

void halo_engine::execute_command(const char* command)
{
	__asm {
		mov edi, command
		push edi
		call halo::function::EXECUTE_COMMAND
		add esp, 4
	}
}

void halo_engine::post_tick()
{
	if (autoCoreSave &&
		*halo::addr::SIMULATION_TICK > 0 &&
		*halo::addr::SIMULATION_TICK % autoCoreSaveTickInterval == 0) {
		core_cache.add_to_cache(*halo::addr::SIMULATION_TICK,
			(const char*)halo::addr::RUNTIME_DATA_BEGIN, halo::constants::CORE_SAVE_SIZE);
	}
	if (queueFullCoreSave) {
		core_cache.add_to_cache(*halo::addr::SIMULATION_TICK,
			(const char*)halo::addr::RUNTIME_DATA_BEGIN, halo::constants::CORE_SAVE_SIZE);
		queueFullCoreSave = false;
	}
	if (queueFullCoreLoad) {
		if (coreLoadStage == 0) {
			bool map_load_needed = core_cache.load_cache_entry_stage1(queueFullCoreLoadTick);

			if (!map_load_needed) {
				core_cache.load_cache_entry_stage2(queueFullCoreLoadTick);
				queueFullCoreLoad = false;
				coreLoadStage = -1;
			}
		}

		coreLoadStage++;
		if (coreLoadStage == 100) {
			core_cache.load_cache_entry_stage2(queueFullCoreLoadTick);
			queueFullCoreLoad = false;
			coreLoadStage = 0;
		}
	}
}

void halo_engine::pre_frame()
{
	const int MIN_RENDERED_FRAMES = 60;
	if (fastForwardTick > 0 && fastForwardTick > * SIMULATION_TICK) {
		enableFastForward = 1;
		if (fastForwardTick < *SIMULATION_TICK + MIN_RENDERED_FRAMES) {
			enable_render();
		}
		else {
			disable_render();
		}
		isPresentEnabled = false;

		if (*SIMULATION_TICK > 100) {
			disable_sound();
		}
	}
	else if (fastForwardTick > 0 && *SIMULATION_TICK == fastForwardTick) {
		*GAME_SPEED = 0;
		enableFastForward = 0;
		fastForwardTick = 0;
		enable_render();
		isPresentEnabled = true;
		enable_sound();
	}
	else {
		enableFastForward = 0;
		fastForwardTick = 0;
		enable_render();
		isPresentEnabled = true;
		enable_sound();
	}
}

// Gets the name of a BSP from the level name and index
std::string halo_engine::current_bsp_name()
{
	std::string currentMap = std::string(MAP_STRING);

	auto keyVal = halo::LEVEL_BSP_NAME.find(std::make_pair(currentMap, *CURRENT_BSP_INDEX));
	if (keyVal != halo::LEVEL_BSP_NAME.end()) {
		return keyVal->second;
	}

	return std::string();
}

halo::MAP halo_engine::current_map()
{
	using halo::MAP;
	std::string currentMap = std::string(MAP_STRING);

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
	switch (*GAME_DIFFICULTY_ACTUAL) {
	case 0: return DIFFICULTY::EASY;
	case 1: return DIFFICULTY::NORMAL;
	case 2: return DIFFICULTY::HEROIC;
	case 3: return DIFFICULTY::LEGENDARY;
	default: return DIFFICULTY::INVALID_DIFFICULTY;
	}
}

void halo_engine::mouse_directinput_override_disable()
{
	patch_memory(PATCH_DINPUT_MOUSE_FUNC, PATCH_DINPUT_MOUSE_ORIGINAL, 7);
}

void halo_engine::mouse_directinput_override_enable()
{
	patch_memory(PATCH_DINPUT_MOUSE_FUNC, PATCH_DINPUT_MOUSE_BYTES, 7);
}

void halo_engine::disable_cutscene_fps_cap()
{
	patch_memory(CUTSCENE_FPS_CAP_PATCH, halo::constants::PATCH_CUTSCENE_FPS_CAP, sizeof(halo::constants::PATCH_CUTSCENE_FPS_CAP));
}

void halo_engine::enable_cutscene_fps_cap()
{
	patch_memory(CUTSCENE_FPS_CAP_PATCH, halo::constants::PATCH_CUTSCENE_FPS_CAP_ORIGINAL, sizeof(halo::constants::PATCH_CUTSCENE_FPS_CAP_ORIGINAL));
}

void halo_engine::disable_look_centering()
{
	patch_memory(LOOK_CENTERING_PATCH, halo::constants::PATCH_LOOK_CENTERING, sizeof(halo::constants::PATCH_LOOK_CENTERING));
}

void halo_engine::enable_look_centering()
{
	patch_memory(LOOK_CENTERING_PATCH, halo::constants::PATCH_LOOK_CENTERING_ORIGINAL, sizeof(halo::constants::PATCH_LOOK_CENTERING_ORIGINAL));
}

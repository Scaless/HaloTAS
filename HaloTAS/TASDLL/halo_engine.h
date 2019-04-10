#pragma once

#include <dwmapi.h>
#include <string>
#include <vector>
#include "halo_constants.h"
#include "gameobject.h"

struct engine_snapshot {
	std::vector<GameObject*> gameObjects;
};

class halo_engine
{
private:
	HWND haloHWND = NULL;
	std::vector<DataPool*> dataPools;
	DataPool* objectDataPool = nullptr;

	void patch_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size);
	void patch_frame_start_func();
	void patch_tick_start_func();

	void unpatch_frame_start_func();
	void unpatch_tick_start_func();

public:
	halo_engine();

	void initialize();
	void cleanup();

	void set_window_handle(HWND handle);
	void update_window_handle();
	HWND window_handle();

	void get_snapshot(engine_snapshot& snapshot);

	// Engine Functions
	void print_hud_text(const std::wstring& input);

	// Patching
	void mouse_directinput_override_disable();
	void mouse_directinput_override_enable();
	
};


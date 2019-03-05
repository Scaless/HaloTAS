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
public:
	halo_engine();
	~halo_engine();

	void update_window_handle();
	HWND window_handle();

	void get_snapshot(engine_snapshot& snapshot);

	// Internal Engine Functions
	void print_hud_text(const std::wstring& input);

	// Patching
	void patch_program_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size);
	void mouse_directinput_override_disable();
	void mouse_directinput_override_enable();
	void inject_frame_start_func();
};


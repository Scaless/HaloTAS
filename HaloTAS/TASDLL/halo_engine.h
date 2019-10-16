#pragma once

#include <dwmapi.h>
#include <string>
#include <vector>
#include "halo_constants.h"
#include "gameobject.h"
#include <atomic>

struct engine_snapshot {
	std::vector<GameObject*> gameObjects;
};

class halo_engine
{
public:
	static halo_engine& get() {
		static halo_engine instance;
		return instance;
	}

private:
	halo_engine();
	~halo_engine();

private:
	HWND haloHWND = NULL;
	std::vector<DataPool*> dataPools;
	DataPool* objectDataPool = nullptr;
	uint32_t enableFastForward{ 0 };
	uint32_t fastForwardTick{ 0 };

	void patch_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size);

	void disable_render();
	void enable_render();

public:
	void set_window_handle(HWND handle);
	void update_window_handle();
	HWND window_handle();
	RECT window_client_rect();
	RECT window_window_rect();
	void focus();

	void get_snapshot(engine_snapshot& snapshot);

	// Engine Functions
	void print_hud_text(const std::wstring& input);
	int get_tag_index_from_path(int, char*);
	void set_debug_camera(bool enabled);
	void fast_forward_to(uint32_t tick);
	void pre_frame();

	std::string current_bsp_name();
	halo::MAP current_map();
	halo::DIFFICULTY current_difficulty();

	// Patching
	void mouse_directinput_override_disable();
	void mouse_directinput_override_enable();

public:
	halo_engine(halo_engine const&) = delete;
	void operator=(halo_engine const&) = delete;
};


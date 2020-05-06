#pragma once

#include <dwmapi.h>
#include <string>
#include <vector>
#include "halo_constants.h"
#include "halo_core_cache.h"
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
	int32_t enableFastForward{ 0 };
	int32_t fastForwardTick{ 0 };
	bool isPresentEnabled{ true };
	bool queueFullCoreSave{ false };
	bool queueFullCoreLoad{ false };
	int32_t queueFullCoreLoadTick = 0;
	bool autoCoreSave{ true };
	int32_t autoCoreSaveTickInterval{ 150 };
	int coreLoadStage{ 0 };
	halo_core_cache core_cache;

	void patch_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size);

	void disable_render();
	void enable_render();
	void disable_sound();
	void enable_sound();

public:
	void set_window_handle(HWND handle);
	void update_window_handle();
	HWND window_handle();
	RECT window_client_rect();
	RECT window_window_rect();
	void focus();

	bool hac_is_loaded();

	void get_snapshot(engine_snapshot& snapshot);
	bool is_present_enabled();

	// Engine Functions
	void print_hud_text(const std::wstring& input);
	int get_tag_index_from_path(int, char*);
	void set_debug_camera(bool enabled);
	void fast_forward_to(uint32_t tick);

	void map_reset();
	void core_save();
	void core_load();
	void core_save_full();
	void core_load_full(int32_t tick_id);
	std::vector<uint32_t> get_cache_ticks();
	void save_checkpoint();
	void load_checkpoint();
	void execute_command(const char* command);

	void pre_frame();
	void post_tick();

	std::string current_bsp_name();
	halo::MAP current_map();
	halo::DIFFICULTY current_difficulty();

	// Patching
	void mouse_directinput_override_disable();
	void mouse_directinput_override_enable();
	void disable_cutscene_fps_cap();
	void enable_cutscene_fps_cap();
	void disable_look_centering();
	void enable_look_centering();

public:
	halo_engine(halo_engine const&) = delete;
	void operator=(halo_engine const&) = delete;
};


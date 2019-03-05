#pragma once

#include <dwmapi.h>
#include <string>

struct engine_snapshot {
	// Camera

};

class halo_engine
{
private:
	HWND haloHWND = NULL;


public:
	halo_engine();
	~halo_engine();

	void update_window_handle();
	HWND window_handle();

	void get_snapshot(engine_snapshot& ss_ref);

	// Internal Engine Functions
	void print_hud_text(const std::wstring& input);

	// Patching
	void patch_program_memory(LPVOID dest_address, uint8_t* src_address, size_t patch_size);
	void mouse_directinput_override_disable();
	void mouse_directinput_override_enable();
};


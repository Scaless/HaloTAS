#include "halo_engine.h"
#include "halo_constants.h"

void halo_engine::update_window_handle()
{
	haloHWND = FindWindowA(nullptr, "Halo");
}

void halo_engine::patch_program_memory(LPVOID dest_address, uint8_t * src_address, size_t patch_size)
{
	unsigned long old_protection, unused;
	//give that address read and write permissions and store the old permissions at oldProtection
	VirtualProtect(dest_address, patch_size, PAGE_EXECUTE_READWRITE, &old_protection);

	//write the memory into the program and overwrite previous value
	std::copy_n(src_address, patch_size, static_cast<uint8_t*>(dest_address));

	//reset the permissions of the address back to oldProtection after writting memory
	VirtualProtect(dest_address, patch_size, old_protection, &unused);
}

halo_engine::halo_engine()
{

}

halo_engine::~halo_engine()
{

}

HWND halo_engine::window_handle()
{
	return haloHWND;
}

void halo_engine::get_snapshot(engine_snapshot& ss_ref)
{

}

void halo_engine::print_hud_text(const std::wstring& input)
{
	wchar_t copy[64];
	wcscpy_s(copy, 64, input.c_str());

	__asm {
		lea	eax, copy
		push eax
		mov	eax, 0 // player index
		call PRINT_HUD_FUNC_PTR
		add	esp, 4
	}
}

void halo_engine::mouse_directinput_override_disable()
{
	patch_program_memory(ADDR_PATCH_DINPUT_MOUSE, PATCH_DINPUT_MOUSE_ORIGINAL, 7);
}

void halo_engine::mouse_directinput_override_enable()
{
	patch_program_memory(ADDR_PATCH_DINPUT_MOUSE, PATCH_DINPUT_MOUSE_BYTES, 7);
}

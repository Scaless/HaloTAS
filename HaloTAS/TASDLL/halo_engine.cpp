#include "halo_engine.h"

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
	// Scan memory for object pools
	for (uint32_t i = 0; i < TAG_ARRAY_LENGTH_BYTES / 4; i++) {
		if (memcmp(&MAGIC_DATAPOOLHEADER, &(ADDR_TAGS_ARRAY[i]), sizeof(MAGIC_DATAPOOLHEADER)) == 0) {
			DataPool* pool = (DataPool*)(&ADDR_TAGS_ARRAY[i] - 10);
			dataPools.push_back(pool);

			if (std::string(pool->Name) == "object") {
				objectDataPool = pool;
			}
		}
	}
}

halo_engine::~halo_engine()
{

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

void halo_engine::inject_frame_start_func()
{
	auto dllHandle = GetModuleHandle(TEXT("TASDLL"));
	auto addr = reinterpret_cast<int>(GetProcAddress(dllHandle, "_CustomFrameStart@0"));
	PATCH_FRAME_BEGIN_FUNC_BYTES[0] = 0xE8; // x86 CALL
	addr -= (int)ADDR_FRAME_BEGIN_FUNC_OFFSET; // Call location
	memcpy_s(&PATCH_FRAME_BEGIN_FUNC_BYTES[1], sizeof(addr), &addr, sizeof(addr));

	patch_program_memory(ADDR_PATCH_FRAME_BEGIN_JUMP_FUNC, PATCH_FRAME_BEGIN_FUNC_BYTES, 15);
}

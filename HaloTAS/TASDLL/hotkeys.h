#pragma once
#include <Windows.h>
#include <unordered_map>

enum class HOTKEY_ACTION {
	ENGINE_MAP_RESET,
	ENGINE_CORE_SAVE,
	ENGINE_CORE_LOAD,
	ENGINE_SAVE_CHECKPOINT,
	ENGINE_LOAD_CHECKPOINT,

	TAS_PREVIOUS_TICK,
	TAS_NEXT_TICK
};

enum class HOTKEY_DEFAULT : uint8_t {
	TAS_PREVIOUS_TICK = VK_LEFT,
	TAS_NEXT_TICK = VK_RIGHT
};

class hotkeys
{
public:
	static hotkeys& get() {
		static hotkeys instance;
		return instance;
	}
	void operator=(hotkeys const&) = delete;
	hotkeys(hotkeys const&) = delete;
private:
	hotkeys() = default;

	std::unordered_map <HOTKEY_ACTION, uint8_t> assigned_hotkeys;
public:
	
	void load_hotkeys();
	void set_hotkey(HOTKEY_ACTION action, uint8_t vk_code);
	static bool is_hotkey_pressed(HOTKEY_ACTION action);
};


#pragma once
#include <Windows.h>
#include <unordered_map>

enum class HOTKEY_ACTION {
	ENGINE_MAP_RESET,
	ENGINE_CORE_SAVE,
	ENGINE_CORE_LOAD,
	ENGINE_SAVE_CHECKPOINT,
	ENGINE_LOAD_CHECKPOINT,
	ENGINE_PLAY,
	ENGINE_PAUSE,

	TAS_PREVIOUS_TICK,
	TAS_NEXT_TICK,
};

enum class HOTKEY_DEFAULT : uint8_t {
	TAS_PREVIOUS_TICK = VK_LEFT,
	TAS_NEXT_TICK = VK_RIGHT,
	ENGINE_PLAY = VK_DECIMAL,
	ENGINE_PAUSE = VK_NUMPAD0,
	ENGINE_MAP_RESET = VK_ADD
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

	// Action, VK Code
	std::unordered_map <HOTKEY_ACTION, uint8_t> assigned_hotkeys;
	// VK Code, Hold Count
	std::unordered_map <uint8_t, uint8_t> key_pressed;
public:
	
	void load_hotkeys();
	void set_hotkey(HOTKEY_ACTION action, uint8_t vk_code);
	static void check_all_hotkeys();
	static bool is_action_trigger_once(HOTKEY_ACTION action);
	static bool is_action_trigger_held(HOTKEY_ACTION action);
};


#include "hotkeys.h"
#include "tas_options.h"

void hotkeys::load_hotkeys()
{
	if (auto engine_map_reset = tas_options::get_value<uint8_t>("HOTKEY::ENGINE_MAP_RESET")) {
		set_hotkey(HOTKEY_ACTION::ENGINE_MAP_RESET, engine_map_reset.value());
	}
	if (auto engine_core_save = tas_options::get_value<uint8_t>("HOTKEY::ENGINE_CORE_SAVE")) {
		set_hotkey(HOTKEY_ACTION::ENGINE_CORE_SAVE, engine_core_save.value());
	}
	if (auto engine_core_load = tas_options::get_value<uint8_t>("HOTKEY::ENGINE_CORE_LOAD")) {
		set_hotkey(HOTKEY_ACTION::ENGINE_CORE_LOAD, engine_core_load.value());
	}
	if (auto engine_save_checkpoint = tas_options::get_value<uint8_t>("HOTKEY::ENGINE_SAVE_CHECKPOINT")) {
		set_hotkey(HOTKEY_ACTION::ENGINE_SAVE_CHECKPOINT, engine_save_checkpoint.value());
	}
	if (auto engine_load_checkpoint = tas_options::get_value<uint8_t>("HOTKEY::ENGINE_LOAD_CHECKPOINT")) {
		set_hotkey(HOTKEY_ACTION::ENGINE_LOAD_CHECKPOINT, engine_load_checkpoint.value());
	}

	if (auto tas_previous_tick = tas_options::get_value<uint8_t>("HOTKEY::TAS_PREVIOUS_TICK")) {
		set_hotkey(HOTKEY_ACTION::TAS_PREVIOUS_TICK, tas_previous_tick.value());
	}
	else {
		set_hotkey(HOTKEY_ACTION::TAS_PREVIOUS_TICK, (uint8_t)HOTKEY_DEFAULT::TAS_PREVIOUS_TICK);
	}
	if (auto tas_next_tick = tas_options::get_value<uint8_t>("HOTKEY::TAS_NEXT_TICK")) {
		set_hotkey(HOTKEY_ACTION::TAS_NEXT_TICK, tas_next_tick.value());
	}
	else {
		set_hotkey(HOTKEY_ACTION::TAS_NEXT_TICK, (uint8_t)HOTKEY_DEFAULT::TAS_NEXT_TICK);
	}
}

void hotkeys::set_hotkey(HOTKEY_ACTION action, uint8_t vk_code)
{
	if (vk_code == 0) {
		assigned_hotkeys.erase(action);
	} else {
		assigned_hotkeys[action] = vk_code;
	}
}

bool hotkeys::is_hotkey_pressed(HOTKEY_ACTION action)
{
	auto& hotkeys = hotkeys::get();

	if (hotkeys.assigned_hotkeys.count(action)) {
		auto vk_code = hotkeys.assigned_hotkeys[action];
		return (GetAsyncKeyState(vk_code) & 0x8000) != 0;
	}

	return false;
}

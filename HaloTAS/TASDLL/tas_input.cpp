#include "tas_input.h"
#include <algorithm>

void tas_input::set_dirty()
{
	dirty = true;
}

tas_input::tas_input()
	: levelName("")
{
}

tas_input::tas_input(std::string fileName)
	: levelName(fileName)
{
}

std::string tas_input::level_name()
{
	return levelName;
}

std::vector<input_moment>* tas_input::input_buffer()
{
	return &inputs;
}

bool tas_input::is_dirty()
{
	return dirty;
}

void tas_input::clear_dirty_flag()
{
	dirty = false;
}

void tas_input::set_inputs(const std::vector<input_moment>& newInputs)
{
	inputs.clear();
	inputs = newInputs;
}

void tas_input::set_kb_input(int32_t tick, halo::KEYS key, uint8_t value)
{
	if (inputs.size() > tick) {
		inputs[tick].inputBuf[to_underlying(key)] = value;
	}
	set_dirty();
}

void tas_input::set_view_angle(int32_t tick, float pitch, float yaw)
{
	if (inputs.size() > tick) {
		inputs[tick].cameraPitch = pitch;
		inputs[tick].cameraYaw = yaw;
	}
	set_dirty();
}

void tas_input::set_pitch(int32_t tick, float pitch)
{
	if (inputs.size() > tick) {
		inputs[tick].cameraPitch = pitch;
	}
	set_dirty();
}

void tas_input::set_yaw(int32_t tick, float yaw)
{
	if (inputs.size() > tick) {
		inputs[tick].cameraYaw = yaw;
	}
	set_dirty();
}

void tas_input::set_mouse_lmb(int32_t tick, bool buttonDown)
{
	if (inputs.size() > tick) {
		inputs[tick].leftMouse = buttonDown ? 1 : 0;
	}
	set_dirty();
}

void tas_input::set_mouse_mmb(int32_t tick, bool buttonDown)
{
	if (inputs.size() > tick) {
		inputs[tick].middleMouse = buttonDown ? 1 : 0;
	}
	set_dirty();
}

void tas_input::set_mouse_rmb(int32_t tick, bool buttonDown)
{
	if (inputs.size() > tick) {
		inputs[tick].rightMouse = buttonDown ? 1 : 0;
	}
	set_dirty();
}

void tas_input::append_tick()
{
	// TODO
	set_dirty();
}

void tas_input::remove_tick_range(size_t tick_begin, size_t tick_end)
{
	if (tick_end < tick_begin) {
		return;
	}

	auto length = inputs.size();
	auto begin_pos = std::min(tick_begin, length);
	auto end_pos = std::min(tick_end+1, length);
	if (length > 0) {
		inputs.erase(inputs.begin() + begin_pos, inputs.begin() + end_pos);
	}
	set_dirty();
}

void tas_input::insert_tick_range(int32_t tick_start, int32_t count)
{
	input_moment newInput = input_moment();
	inputs.insert(inputs.begin() + tick_start, count, newInput);
	set_dirty();
}

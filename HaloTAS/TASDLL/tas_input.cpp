#include "tas_input.h"
#include <algorithm>

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
}

void tas_input::set_view_angle(int32_t tick, float pitch, float yaw)
{
	if (inputs.size() > tick) {
		inputs[tick].cameraPitch = pitch;
		inputs[tick].cameraYaw = yaw;
	}
}

void tas_input::set_pitch(int32_t tick, float pitch)
{
	if (inputs.size() > tick) {
		inputs[tick].cameraPitch = pitch;
	}
}

void tas_input::set_yaw(int32_t tick, float yaw)
{
	if (inputs.size() > tick) {
		inputs[tick].cameraYaw = yaw;
	}
}

void tas_input::set_mouse_input(int32_t tick, bool leftMouseDown, bool rightMouseDown)
{
	// TODO
}

void tas_input::set_breakpoint(int32_t tick, bool enabled)
{
	// TODO
}

void tas_input::append_tick()
{
	// TODO
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
}

void tas_input::insert_tick_range(int32_t tick_start, int32_t count)
{
	input_moment newInput = input_moment();
	inputs.insert(inputs.begin() + tick_start, count, newInput);
}

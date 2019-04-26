#include "tas_input.h"

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

void tas_input::set_kb_input(int32_t tick, KEYS key, uint8_t value)
{
	if (inputs.size() > tick) {
		inputs[tick].inputBuf[key] = value;
	}
	// TODO
}

void tas_input::set_view_angle(int32_t tick, float pitch, float yaw)
{
	// TODO
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

void tas_input::remove_tick_range(int32_t tick_begin, int32_t tick_end)
{
	// TODO Bounds checking
	inputs.erase(inputs.begin() + tick_begin, inputs.begin() + tick_end + 1);
}

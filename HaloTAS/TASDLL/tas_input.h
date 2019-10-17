#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "halo_constants.h"

struct input_moment {
	uint32_t tick;
	uint8_t inputBuf[to_underlying(halo::KEYS::KEY_COUNT)];
	int32_t inputMouseX, inputMouseY;
	float cameraYaw, cameraPitch;
	glm::vec3 cameraLocation;
	uint8_t leftMouse, middleMouse, rightMouse;
	int32_t rng;
};

// Manages input for a level
class tas_input {
	std::string levelName;
	std::vector<input_moment> inputs;

public:
	tas_input();
	tas_input(std::string fileName);

	std::string level_name();
	std::vector<input_moment>* input_buffer();

	void set_inputs(const std::vector<input_moment>& newInputs);

	// Set kb input for tick
	void set_kb_input(int32_t tick, halo::KEYS key, uint8_t value);
	// Set view angle
	void set_view_angle(int32_t tick, float pitch, float yaw);
	void set_pitch(int32_t tick, float pitch);
	void set_yaw(int32_t tick, float yaw);
	// Set mouse input for tick
	void set_mouse_input(int32_t tick, bool leftMouseDown, bool rightMouseDown);
	// Set breakpoint on tick
	void set_breakpoint(int32_t tick, bool enabled);

	// Appends tick to the end of the current input range
	void append_tick();
	// Removes ticks from the current input range
	void remove_tick_range(size_t tick_begin, size_t tick_end);
	// Inserts count # of ticks after the starting tick location
	void insert_tick_range(int32_t tick_start, int32_t count);
};
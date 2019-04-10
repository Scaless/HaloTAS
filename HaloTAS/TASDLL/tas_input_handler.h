#pragma once

#include <Windows.h>
#include <vector>
#include <glm/glm.hpp>

struct input_moment {
	uint32_t tick;
	uint8_t inputBuf[104];
	int32_t inputMouseX, inputMouseY;
	float cameraYaw, cameraPitch;
	glm::vec3 cameraLocation;
	uint8_t leftMouse, middleMouse, rightMouse;
};

class tas_input_handler
{
private:
	std::vector<input_moment> inputs;
	void set_engine_run_frame_begin();

public:
	tas_input_handler();
	~tas_input_handler();

	void set_record(bool newRecord);
	void set_playback(bool newPlayback);

	void load_inputs_current_level();
	void save_inputs_current_level();
	void reload_playback_buffer();
	
	std::vector<input_moment>* getInputs();
};


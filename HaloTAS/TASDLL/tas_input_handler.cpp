#include "tas_input_handler.h"

#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <atomic>
#include "halo_constants.h"

static std::unordered_map<uint64_t, input_moment> stored_inputs_map;
static std::atomic_bool record = false;
static std::atomic_bool playback = false;

void tas_input_handler::set_engine_run_frame_begin()
{
	*ADDR_RUN_FRAME_BEGIN_CODE = record || playback;
}

tas_input_handler::tas_input_handler()
{
}

tas_input_handler::~tas_input_handler()
{
	record = false;
	playback = false;
	set_engine_run_frame_begin();
}

void tas_input_handler::set_record(bool newRecord)
{
	record = newRecord;
	if (record) {
		playback = false;
	}

	set_engine_run_frame_begin();
}

void tas_input_handler::set_playback(bool newPlayback)
{
	playback = newPlayback;
	if (playback) {
		record = false;
	}

	set_engine_run_frame_begin();
}

void tas_input_handler::load_inputs_current_level()
{
	stored_inputs_map.clear();
	
	std::string currentMap{ ADDR_MAP_STRING };
	std::replace(currentMap.begin(), currentMap.end(), '\\', '.');
	
	std::ifstream logFile(currentMap + ".hbin", std::ios::app | std::ios::binary);
	
	char buf[sizeof(input_moment)];
	while(!logFile.eof())
	{
		logFile.read(buf, sizeof(input_moment));
		input_moment* im = reinterpret_cast<input_moment*>(&buf);

		input_key ik;
		ik.subKey.subLevel = im->checkpointId;
		ik.subKey.inputCounter = im->tick;

		if (stored_inputs_map.find(ik.fullKey) != stored_inputs_map.end()) {
			std::string message = "File: " + currentMap + ".hbin\n" +
				"Tick: " + std::to_string(ik.fullKey) + "\n" +
				"Found duplicate input sequences for the same tick. Only one input sequence per tick allowed." +
				"Alter the HBIN so that only one input line per tick exists.";

			MessageBoxA(NULL, 
				message.c_str(),
				"Aborting Load: Multiple Input Sequences Found", MB_ICONWARNING | MB_OK);
			stored_inputs_map.clear();
			return;
		}

		stored_inputs_map[ik.fullKey] = *im;
	}
}


extern "C" __declspec(dllexport) void WINAPI CustomFrameStart() {
	static int32_t last_input_tick;
	static uint32_t last_frame;

	const uint32_t frames = *ADDR_FRAMES_SINCE_LEVEL_START;
	const int32_t tick = *ADDR_SIMULATION_TICK;
	const uint32_t subLevel = *ADDR_CHECKPOINT_INDICATOR;

	if (playback) {
		*ADDR_DINPUT_MOUSEX = 0;
		*ADDR_DINPUT_MOUSEY = 0;
	}

	if (tick == last_input_tick)
		return;

	last_input_tick = tick;

	if (record)
	{
		std::string currentMap{ ADDR_MAP_STRING };
		std::replace(currentMap.begin(), currentMap.end(), '\\', '.');
		std::ofstream logFile(currentMap + ".hbin", std::ios::app | std::ios::binary); // levels.a10.a10.hbin

		input_moment im;
		for (auto i = 0; i < sizeof(im.inputBuf); i++) {
			im.inputBuf[i] = ADDR_KEYBOARD_INPUT[i];
		}
		im.checkpointId = *ADDR_CHECKPOINT_INDICATOR;
		im.tick = tick;
		//im.inputMouseX = *ADDR_DINPUT_MOUSEX;
		//im.inputMouseY = *ADDR_DINPUT_MOUSEY;
		im.cameraYaw = *ADDR_PLAYER_YAW_ROTATION_RADIANS;
		im.cameraPitch = *ADDR_PLAYER_PITCH_ROTATION_RADIANS;
		im.leftMouse = *ADDR_LEFTMOUSE;
		im.middleMouse = *ADDR_MIDDLEMOUSE;
		im.rightMouse = *ADDR_RIGHTMOUSE;
		im.cameraLocation = *ADDR_CAMERA_POSITION;

		logFile.write(reinterpret_cast<char*>(&im), sizeof(im));
		logFile.close();
	}

	if (playback)
	{
		input_key ik;
		ik.subKey.subLevel = subLevel;
		ik.subKey.inputCounter = tick;
		if (stored_inputs_map.count(ik.fullKey))
		{
			input_moment savedIM = stored_inputs_map[ik.fullKey];

			memcpy(ADDR_KEYBOARD_INPUT, savedIM.inputBuf, sizeof(savedIM.inputBuf));
			*ADDR_PLAYER_YAW_ROTATION_RADIANS = savedIM.cameraYaw;
			*ADDR_PLAYER_PITCH_ROTATION_RADIANS = savedIM.cameraPitch;
			//*ADDR_DINPUT_MOUSEX = savedIM.inputMouseX;
			//*ADDR_DINPUT_MOUSEY = savedIM.inputMouseY;
			*ADDR_LEFTMOUSE = savedIM.leftMouse;
			*ADDR_MIDDLEMOUSE = savedIM.middleMouse;
			*ADDR_RIGHTMOUSE = savedIM.rightMouse;
			//*ADDR_CAMERA_POSITION = savedIM.cameraLocation;

			//drift = glm::distance(*ADDR_CAMERA_POSITION,savedIM.cameraLocation);
		}
	}
}
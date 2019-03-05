
#include <Windows.h>
#include <string>
#include <map>
#include <set>
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <dwmapi.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <vector>
#include <atomic>
#include <unordered_set>
#include <algorithm>

#include "halo_engine.h"
#include "halo_constants.h"
#include "gameobject.h"
#include "render_text.h"
#include "render_cube.h"
#include "render_opengl.h"
#include "tas_overlay.h"
#include "livesplit.h"

struct InputMoment {
	uint32_t checkpointId;
	uint32_t tick;
	uint8_t inputBuf[104];
	int32_t inputMouseX, inputMouseY;
	float cameraYaw, cameraPitch;
	glm::vec3 cameraLocation;
	uint8_t leftMouse,middleMouse,rightMouse;
};

union InputKey {
	uint64_t fullKey;
	struct {
		uint32_t subLevel;
		uint32_t inputCounter;
	} subKey;
};

static bool record = false;
static bool playback = false;
static int32_t last_input_tick;
static uint32_t last_frame;

static std::map<uint64_t, InputMoment> stored_inputs_map;
//static float drift = 0;

extern "C" __declspec(dllexport) void WINAPI CustomFrameStart() {

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

		InputMoment im;
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

	if(playback)
	{
		InputKey ik;
		ik.subKey.subLevel = subLevel;
		ik.subKey.inputCounter = tick;
		if(stored_inputs_map.count(ik.fullKey))
		{
			InputMoment savedIM = stored_inputs_map[ik.fullKey];

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

void load_inputs()
{
	stored_inputs_map.clear();
	
	std::string currentMap{ ADDR_MAP_STRING };
	std::replace(currentMap.begin(), currentMap.end(), '\\', '.');

	std::ifstream logFile(currentMap + ".hbin", std::ios::app | std::ios::binary);
	
	char buf[sizeof(InputMoment)];
	while(!logFile.eof())
	{
		logFile.read(buf, sizeof(InputMoment));
		InputMoment* im = reinterpret_cast<InputMoment*>(&buf);

		InputKey ik;
		ik.subKey.subLevel = im->checkpointId;
		ik.subKey.inputCounter = im->tick;
		stored_inputs_map[ik.fullKey] = *im;
	}
}

void Run() {
	std::unique_ptr<halo_engine> engine(new halo_engine);
	std::unique_ptr<livesplit> livesplit(new livesplit);
	std::unique_ptr<tas_overlay> overlay(new tas_overlay);

	engine->inject_frame_start_func();

	bool close = false;

	engine->update_window_handle();

	// Main loop
	while (!close)
	{
		livesplit_export newExport = {};
		strcpy_s((char*)&newExport.currentMap, sizeof(newExport.currentMap), ADDR_MAP_STRING);
		livesplit->update_export(newExport);

		auto input = overlay->render_and_get_input(*engine);

		if (input.loadPlayback) {
			load_inputs();
		}

		if (input.overlayClose) {
			break;
		}

		record = input.record;
		playback = input.playback;
	}

}

DWORD WINAPI Main_Thread(HMODULE hDLL)
{
	// Setup GLFW
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		return 1;
	}
	
	// Main execution happens in Run() so that destructors are called properly
	// before FreeLibraryAndExitThread gets called
	Run();

	// Cleanup GLFW
	glfwTerminate();

	FreeLibraryAndExitThread(hDLL, NULL);
}

// Entry point for DLL
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID /*Reserved*/)
{
	DWORD dwThreadID;
	if (Reason == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)Main_Thread, hDLL, 0, &dwThreadID);
	}
	return TRUE;
}

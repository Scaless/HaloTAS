
#include <chrono>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "halo_engine.h"
#include "tas_overlay.h"
#include "tas_info_window.h"
#include "tas_input_handler.h"
#include "livesplit.h"

void run() {
	auto engine = std::make_unique<halo_engine>();
	auto liveSplit = std::make_unique<livesplit>();
	auto overlay = std::make_unique<tas_overlay>();
	auto infoWindow = std::make_unique<tas_info_window>();
	auto inputHandler = std::make_unique<tas_input_handler>();

	engine->patch_frame_start_func();
	engine->patch_tick_start_func();
	engine->update_window_handle();

	bool close = false;
	auto lastUpdate = std::chrono::system_clock::now();

	// Main loop
	while (!close)
	{
		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate) > std::chrono::milliseconds(100)) {
			engine->update_window_handle();
		}

		livesplit_export newExport = {};
		strcpy_s((char*)&newExport.currentMap, sizeof(newExport.currentMap), ADDR_MAP_STRING);
		liveSplit->update_export(newExport);

		infoWindow->render(*engine);
		auto input = infoWindow->getInput();

		if (input.loadPlayback) {
			inputHandler->load_inputs_current_level();
		}
		if (input.overlayClose) {
			close |= true;
		}

		overlay->render(*engine, input.overlayOptions);

		inputHandler->set_record(input.record);
		inputHandler->set_playback(input.playback);

		close |= infoWindow->shouldClose();
	}
}


DWORD WINAPI Main_Thread(HMODULE hDLL)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		return 1;
	}

	// Main execution happens in run() so that destructors are called properly
	// before FreeLibraryAndExitThread gets called
	run();

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

int main() {
	run();
}
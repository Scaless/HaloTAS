
#include <chrono>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "globals.h"
#include "halo_engine.h"
#include "tas_overlay.h"
#include "tas_info_window.h"
#include "tas_input_handler.h"
#include "livesplit.h"

void run() {
	gEngine = std::make_unique<halo_engine>();
	gInputHandler = std::make_unique<tas_input_handler>();

	auto liveSplit = std::make_unique<livesplit>();
	auto overlay = std::make_unique<tas_overlay>();
	auto infoWindow = std::make_unique<tas_info_window>();

	gEngine->initialize();

	auto lastEngineUpdate = std::chrono::system_clock::now();
	auto lastDisplayUpdate = std::chrono::system_clock::now();
	bool close = false;

	// Main loop
	while (!close)
	{
		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEngineUpdate) > std::chrono::milliseconds(100)) {
			gEngine->update_window_handle();
			lastEngineUpdate = now;
		}

		// Keep people honest
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastDisplayUpdate) > std::chrono::seconds(10)) {
			gEngine->print_hud_text(L"Official runs are invalid while HaloTAS is running!");
			lastDisplayUpdate = now;
		}

		livesplit_export newExport = {};
		strcpy_s((char*)&newExport.currentMap, sizeof(newExport.currentMap), ADDR_MAP_STRING);
		liveSplit->update_export(newExport);

		auto input = infoWindow->getInput();
		if (input.loadPlayback) {
			gInputHandler->load_inputs_current_level();
		}
		if (input.overlayClose) {
			close |= true;
		}

		infoWindow->render();
		overlay->render(input.overlayOptions);

		gInputHandler->set_record(input.record);
		gInputHandler->set_playback(input.playback);

		close |= infoWindow->shouldClose();
	}

	gEngine->cleanup();
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

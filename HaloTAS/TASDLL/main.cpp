
#define DIRECTINPUT_VERSION 0x0800

#include <chrono>
#include <iostream>
#include <thread>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "halo_engine.h"
#include "tas_overlay.h"
#include "tas_info_window.h"
#include "tas_input_handler.h"
#include "tas_options.h"
#include "tas_logger.h"
#include "tas_hooks.h"
#include "livesplit.h"
#include "helpers.h"
#include "render_d3d9.h"
#include "hotkeys.h"
#include "randomizer.h"
#include <mmsystem.h>
#include <shellapi.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

std::unique_ptr<tas_info_window> infoWindow{ nullptr };

void run() {

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		tas_logger::fatal("Failed to initialize GLFW");
		return;
	}

	auto& gEngine = halo_engine::get();
	auto& gInputHandler = tas_input_handler::get();
	auto& gHotkeys = hotkeys::get();

	gHotkeys.load_hotkeys();
	gInputHandler.get_inputs_from_files();

	auto liveSplit = std::make_unique<livesplit>();
	auto overlay = std::make_unique<tas_overlay>();
	infoWindow = std::make_unique<tas_info_window>();

	auto lastEngineUpdate = std::chrono::system_clock::now();
	auto lastDisplayUpdate = std::chrono::system_clock::now();
	bool close = false;

	// Main loop
	while (!close)
	{
		glfwPollEvents();
		gHotkeys.check_all_hotkeys();

		auto now = std::chrono::system_clock::now();
		auto engineUpdateDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEngineUpdate);
		if (engineUpdateDuration > std::chrono::seconds(10)) {
			gEngine.update_window_handle();
			lastEngineUpdate = now;
		}
		
		livesplit_export newExport = {};
		strcpy_s((char*)&newExport.currentMap, sizeof(newExport.currentMap), halo::addr::MAP_STRING);
		liveSplit->update_export(newExport);

		auto input = infoWindow->getInput();
		if (input.loadPlayback) {
			gInputHandler.get_inputs_from_files();
		}
		if (input.overlayClose) {
			close |= true;
		}

		infoWindow->render();
		overlay->render(input.overlayOptions);

		gInputHandler.set_record(input.record);
		gInputHandler.set_playback(input.playback);
		gInputHandler.autosave_check();

		close |= infoWindow->shouldClose();
	}

	gEngine.mouse_directinput_override_disable();
	glfwTerminate();

}

bool version_supported() {
	std::string currentVersion{ halo::addr::GAME_VERSION };
	bool version_supported = (currentVersion == "01.00.10.0621");
	if (!version_supported) {
		std::stringstream ss;
		ss << "The version of halo currently running (" << currentVersion << ") is not supported by HaloTAS.\r\n";
		ss << "Click YES to open a link to the 1.0.10 patch in your browser, otherwise click NO to close HaloTAS.";
		if (MessageBox(NULL, ss.str().c_str(), "HaloTAS: Halo Version Unsupported", MB_YESNO) == IDYES) {
			ShellExecute(0, 0, "https://www.bungie.net/en-US/Forums/Post/64943622", 0, 0, SW_SHOW);
		}
		return version_supported;
	}
	return version_supported;
}

const std::vector<std::string> REQUIRED_PATHS = {
	"HaloTASFiles",
	"HaloTASFiles/Recordings",
	"HaloTASFiles/Recordings/_Autosave",
	"HaloTASFiles/Plugins",
	"HaloTASFiles/Patches",
	"HaloTASFiles/Cache",
	"HaloTASFiles/Saves",
};

bool verify_file_structure() {
	bool path_error = false;
	for (auto& path : REQUIRED_PATHS) {
		if (!std::filesystem::exists(path)) {
			if (!std::filesystem::create_directories(path)) {
				// Directory wasnt created
				tas_logger::warning("Failed to create directory (%s)", path.c_str());
				path_error = true;
			}
		}
	}
	return path_error;
}

DWORD WINAPI MainThread(HMODULE hDLL)
{
	try {
		verify_file_structure();

		tas_logger::info("===== HaloTAS Started =====");
		
		if (!std::filesystem::exists("HaloTASFiles/tas.bin")) {
			std::string message = "Could not find tas.bin. Make sure to copy the HaloTASFiles folder to your Halo directory.";
			MessageBox(NULL, message.c_str(), "File Missing", MB_OK);
			tas_logger::fatal("Could not find tas.bin");
			FreeLibraryAndExitThread(hDLL, NULL);
		}

		tas_logger::info("Current working directory: %s", std::filesystem::current_path().string().c_str());

		// Wait for halo.exe to init otherwise we may access invalid memory
		while (*halo::addr::SIMULATION_TICK <= 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		// Check to make sure user is running a supported version
		if (!version_supported()) {
			tas_logger::fatal("Unsupported version of halo.exe");
			FreeLibraryAndExitThread(hDLL, NULL);
		}

		auto& gInputHandler = tas_input_handler::get();
		auto& gRandomizer = randomizer::get();
		auto& gEngine = halo_engine::get();
		auto& d3d9 = render_d3d9::get();

		auto tasHooks = std::make_unique<tas_hooks>();
		tasHooks->attach_all();
		run();
		tasHooks->detach_all();

		tas_logger::info("===== HaloTAS Closed =====");
	}
	catch (const std::exception & e) {
		auto& gEngine = halo_engine::get();
		gEngine.mouse_directinput_override_disable();
		tas_logger::fatal("%s", e.what());
	}
	tas_logger::flush_and_exit();
	FreeLibraryAndExitThread(hDLL, NULL);
}

// Entry point for DLL
INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID /*Reserved*/)
{
	DWORD dwThreadID;
	if (Reason == DLL_PROCESS_ATTACH) {
		CreateThread(0, 0x1000, (LPTHREAD_START_ROUTINE)MainThread, hDLL, 0, &dwThreadID);
	}
	return TRUE;
}

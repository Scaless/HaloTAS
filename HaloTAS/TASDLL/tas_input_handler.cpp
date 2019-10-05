#include "tas_input_handler.h"

#include <boost/algorithm/string/predicate.hpp> // for ends_with
#include <boost/circular_buffer.hpp>
#include <vector>
#include <queue>
#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>
#include <atomic>
#include "halo_constants.h"
#include "helpers.h"

using std::vector, std::string;

// TODO: replace with tas_input object 
static std::vector<input_moment> playback_buffer_current_level;
static std::atomic_bool record = false;
static std::atomic_bool playback = false;

static std::string last_level_name;
static int32_t last_input_tick;

static int32_t rng_count_since_last_tick;
static int32_t last_rng;
static boost::circular_buffer<float> rng_count_histogram_buffer(256);

void tas_input_handler::set_engine_run_frame_begin()
{
	*ADDR_RUN_FRAME_BEGIN_CODE = record || playback;
}

tas_input_handler::tas_input_handler()
{
	set_engine_run_frame_begin();
}

void tas_input_handler::set_record(bool newRecord)
{
	record = newRecord;

	set_engine_run_frame_begin();
}

void tas_input_handler::set_playback(bool newPlayback)
{
	playback = newPlayback;

	set_engine_run_frame_begin();
}

void tas_input_handler::load_input_from_file(std::filesystem::path filePath) {
	if (!std::filesystem::exists(filePath)) {
		return;
	}

	tas_input levelInput(filePath.filename().string());

	std::vector<input_moment> inputs;
	auto estimatedElements = std::filesystem::file_size(filePath) / sizeof(input_moment) + 1;
	inputs.reserve(estimatedElements);

	std::ifstream logFile(filePath.string(), std::ios::in | std::ios::binary);

	char buf[sizeof(input_moment)];
	while (!logFile.eof())
	{
		logFile.read(buf, sizeof(input_moment));
		if (!logFile) {
			break;
		}

		input_moment* im = reinterpret_cast<input_moment*>(&buf);

		inputs.push_back(*im);
	}

	levelInput.set_inputs(inputs);
	
	levelInputs[filePath.filename().string()] = levelInput;
}

void tas_input_handler::get_inputs_from_files()
{
	levelInputs.clear();
	for (const auto& file : std::filesystem::directory_iterator("")) {
		if (boost::algorithm::ends_with(file.path().string(), ".hbin")) {
			load_input_from_file(file.path());
		}
	}
}

void tas_input_handler::save_inputs()
{
	for (auto& inputSet : levelInputs) {
		std::ofstream logFile(inputSet.first, std::ios::trunc | std::ios::binary);

		for (auto& inp : *inputSet.second.input_buffer()) {
			logFile.write(reinterpret_cast<char*>(&inp), sizeof(inp));
		}

		logFile.close();
	}
}

void tas_input_handler::reload_playback_buffer(tas_input* input) {

	playback_buffer_current_level = *input->input_buffer();
	//playback_buffer_current_level = inputs;
}


void tas_input_handler::pre_tick()
{
	const int32_t tick = *ADDR_SIMULATION_TICK;

	if (tick == 0) {
		*ADDR_HUD_TIMER_PAUSED = 0;
		*ADDR_HUD_TIMER_VISIBLE = 1;
		*ADDR_HUD_TIMER_TOTAL_TIME_TICKS = 18000;
		*ADDR_HUD_TIMER_LOCATION = HUD_LOCATION::TOP_RIGHT;
		*ADDR_HUD_TIMER_OFFSET_X = 0;
		*ADDR_HUD_TIMER_OFFSET_Y = 50;
	}

	
	/*if(ADDR_KEYBOARD_INPUT[KEYS::Tab] == 1)
		ADDR_KEYBOARD_INPUT[KEYS::Tab] = 2;
	else
		ADDR_KEYBOARD_INPUT[KEYS::Tab] = 1;
*/
	if (playback) {
		*ADDR_DINPUT_MOUSEX = 0;
		*ADDR_DINPUT_MOUSEY = 0;
	}

	if (last_level_name.compare(ADDR_MAP_STRING) != 0 || tick == 0) {
		last_level_name = ADDR_MAP_STRING;
		tas_input_handler::inputTickCounter = 0;
		rng_count_histogram_buffer.clear();
	}

	if (playback)
	{
		if (playback_buffer_current_level.size() > tas_input_handler::inputTickCounter) {
			input_moment savedIM = playback_buffer_current_level[tas_input_handler::inputTickCounter];

			memcpy(ADDR_KEYBOARD_INPUT, savedIM.inputBuf, sizeof(savedIM.inputBuf));
			*ADDR_PLAYER_YAW_ROTATION_RADIANS = savedIM.cameraYaw;
			*ADDR_PLAYER_PITCH_ROTATION_RADIANS = savedIM.cameraPitch;
			*ADDR_LEFTMOUSE = savedIM.leftMouse;
			*ADDR_RIGHTMOUSE = savedIM.rightMouse;

			if (savedIM.middleMouse == 0) {
				*ADDR_MIDDLEMOUSE = 0;
			}
			else {
				if (*ADDR_MIDDLEMOUSE == 0) {
					*ADDR_MIDDLEMOUSE = 1;
				}
			}

			//*ADDR_CAMERA_POSITION = savedIM.cameraLocation;
			//*ADDR_DINPUT_MOUSEX = savedIM.inputMouseX;
			//*ADDR_DINPUT_MOUSEY = savedIM.inputMouseY;
			//drift = glm::distance(*ADDR_CAMERA_POSITION,savedIM.cameraLocation);
		}

		// Fix for enter being stuck held down
		/*if (tas_input_handler::inputTickCounter > 0) {
			if (playback_buffer_current_level.size() > tas_input_handler::inputTickCounter - 1) {
				if (playback_buffer_current_level[tas_input_handler::inputTickCounter - 1].inputBuf[KEYS::Enter] > 0) {
					ADDR_KEYBOARD_INPUT[Enter] = 0;
				}
			}
		}*/
	}

	tas_input_handler::inputTickCounter += 1;

	rng_count_since_last_tick = calc_rng_count(last_rng, *ADDR_RNG, 5000);
	last_rng = *ADDR_RNG;
	rng_count_histogram_buffer.push_back(rng_count_since_last_tick);
}

static int recordedTick = 0;
void tas_input_handler::post_tick()
{
	const int32_t tick = *ADDR_SIMULATION_TICK - 1;

	if (tick == 0) {
		recordedTick = 0;
	}

	if (tick > 0) {
		*ADDR_HUD_TIMER_VISIBLE = 1;
		*ADDR_HUD_TIMER_TOTAL_TIME_TICKS = tick;
		*ADDR_HUD_TIMER_START_TICK = tick;
	}
	
	if (playback && *ADDR_SIMULATION_TICK > 10) {
		if (playback_buffer_current_level.size() > recordedTick) {
			input_moment savedIM = playback_buffer_current_level[recordedTick];
			if (*ADDR_RNG == savedIM.rng) {

			}
		}
	}

	if (record && recordedTick > static_cast<int32_t>(playback_buffer_current_level.size()) - 1)
	{
		std::string currentMap{ ADDR_MAP_STRING };
		std::replace(currentMap.begin(), currentMap.end(), '\\', '.');
		std::ofstream logFile(currentMap + ".hbin", std::ios::app | std::ios::binary); // levels.a10.a10.hbin

		input_moment im;
		for (auto i = 0; i < sizeof(im.inputBuf); i++) {
			im.inputBuf[i] = ADDR_KEYBOARD_INPUT[i];
		}
		im.tick = tick;
		im.inputMouseX = *ADDR_DINPUT_MOUSEX;
		im.inputMouseY = *ADDR_DINPUT_MOUSEY;
		im.cameraYaw = *ADDR_PLAYER_YAW_ROTATION_RADIANS;
		im.cameraPitch = *ADDR_PLAYER_PITCH_ROTATION_RADIANS;
		im.leftMouse = *ADDR_LEFTMOUSE;
		im.middleMouse = *ADDR_MIDDLEMOUSE;
		im.rightMouse = *ADDR_RIGHTMOUSE;
		im.cameraLocation = *ADDR_CAMERA_POSITION;
		im.rng = *ADDR_RNG;

		logFile.write(reinterpret_cast<char*>(&im), sizeof(im));
		logFile.close();
	}

	recordedTick++;
}

	static bool justThisOnce = false;
void tas_input_handler::pre_frame()
{
	if (*ADDR_MIDDLEMOUSE == 1) {
		*ADDR_MIDDLEMOUSE = 2;
	}
}

void tas_input_handler::post_frame()
{

}

void tas_input_handler::pre_loop()
{
	// Fix for input buffer overflow
	std::string scan = { 0x2E, 0x00, 0x2E, 0x00, 0x2E, 0x00, 0x20, 0x00 };
	auto* inputSlot = ADDR_INPUT_SLOT;
	for (int i = 0; i < 4; i++) {
		std::string inputBuf(inputSlot, inputSlot + 128);
		std::size_t n = inputBuf.find(scan);
		if (n != std::string::npos && n < 120) {
			inputSlot[n + 6] = 0x2E;
		}
		inputSlot += 140;
	}
}

void tas_input_handler::post_loop()
{

}

std::vector<std::string> tas_input_handler::get_loaded_levels()
{
	// TODO
	vector<string> names;
	names.reserve(levelInputs.size());
	for (const auto& level : levelInputs) {
		names.push_back(level.first);
	}
	return names;
}

tas_input* tas_input_handler::get_inputs(std::string levelName)
{
	if (levelInputs.count(levelName)) {
		return &levelInputs[levelName];
	}
	return nullptr;
}

std::array<float, 256> tas_input_handler::get_rng_histogram_data()
{
	std::array<float, 256> data{};
	
	auto part1 = rng_count_histogram_buffer.array_one();
	auto part2 = rng_count_histogram_buffer.array_two();

	memcpy_s(data.data(), sizeof(data), part1.first, sizeof(data[0]) * part1.second);
	memcpy_s(data.data() + part1.second, sizeof(data) - part1.second * sizeof(data[0]), part2.first, sizeof(data[0]) * part2.second);

	return data;
}

void tas_input_handler::clear_rng_histogram_data()
{
	rng_count_histogram_buffer.clear();
}

void tas_input_handler::insert_dummy_rng_histogram_value()
{
	rng_count_histogram_buffer.push_back(0);
}

int32_t tas_input_handler::get_current_playback_tick()
{
	return inputTickCounter;
}

int32_t tas_input_handler::get_rng_advances_since_last_tick()
{
	return rng_count_since_last_tick;
}

bool tas_input_handler::this_tick_enter()
{
	if (!playback)
		return false;

	if (!(playback_buffer_current_level.size() > tas_input_handler::inputTickCounter))
		return false;

	input_moment savedIM = playback_buffer_current_level[tas_input_handler::inputTickCounter];
	if (savedIM.inputBuf[KEYS::Enter]) {
		return true;
	}
	return false;
}

bool tas_input_handler::this_tick_tab()
{
	if (!playback)
		return false;

	if (!(playback_buffer_current_level.size() > tas_input_handler::inputTickCounter))
		return false;

	input_moment savedIM = playback_buffer_current_level[tas_input_handler::inputTickCounter];
	if (savedIM.inputBuf[KEYS::Tab]) {
		return true;
	}
	return false;
}

bool tas_input_handler::this_tick_g()
{
	if (!playback)
		return false;

	if (!(playback_buffer_current_level.size() > tas_input_handler::inputTickCounter))
		return false;

	input_moment savedIM = playback_buffer_current_level[tas_input_handler::inputTickCounter];
	if (savedIM.inputBuf[KEYS::G]) {
		return true;
	}
	return false;
}

bool tas_input_handler::this_tick_mb(int btn)
{
	if (!playback)
		return false;

	if (!(playback_buffer_current_level.size() > tas_input_handler::inputTickCounter))
		return false;

	input_moment savedIM = playback_buffer_current_level[tas_input_handler::inputTickCounter];

	switch (btn) {
	case 0:
		if (savedIM.leftMouse > 0) {
			return true;
		}
		break;
	case 1:
		if (savedIM.rightMouse > 0) {
			return true;
		}
		break;
	case 2:
		if (savedIM.middleMouse > 0) {
			return true;
		}
		break;
	default:
		return false;
	}

	return false;
}

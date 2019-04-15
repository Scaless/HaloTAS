#pragma once

#include <Windows.h>
#include <filesystem>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "tas_input.h"

class tas_input_handler
{
private:
	std::map<std::string,tas_input> levelInputs;
	//std::vector<input_moment> inputs;
	void set_engine_run_frame_begin();

	void load_input_from_file(std::filesystem::path filePath);
public:
	tas_input_handler();
	~tas_input_handler();

	void set_record(bool newRecord);
	void set_playback(bool newPlayback);

	void get_inputs_from_files();
	void save_inputs();
	void reload_playback_buffer(tas_input* input);

	std::vector<std::string> get_loaded_levels();
	tas_input* get_inputs(std::string levelName);
	int32_t get_current_playback_tick();
};


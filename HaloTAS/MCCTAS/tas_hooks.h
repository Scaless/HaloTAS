#pragma once

#include "pch.h"

class tas_hooks
{
public:
	tas_hooks();
	~tas_hooks();

	void attach_all();
	void detach_all();

	static void execute_halo1_command(const std::string & command);
	static std::vector<std::string> get_sprintf_values();
	static GameEngineType get_loaded_engine();

private:
	void attach_global_hooks();
	void detach_global_hooks();
	void init_global_hooks();
	void detach_temporary_hooks();
};

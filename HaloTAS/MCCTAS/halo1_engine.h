#pragma once
#include "halo1_types.h"

class halo1_engine
{
private:
	static halo1_engine& get() {
		static halo1_engine instance;
		return instance;
	}

	halo1_engine() = default;
	~halo1_engine() = default;

public:
	static void get_game_information(halo::halo1_snapshot& snapshot);

	static void set_skull_enabled(halo::Halo1Skull skull, bool enabled);

	static void execute_command(const char* command);
};


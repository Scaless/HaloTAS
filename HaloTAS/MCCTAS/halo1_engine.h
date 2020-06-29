#pragma once
#include "halo1_types.h"

class halo1_engine
{
public:
	static halo1_engine& get() {
		static halo1_engine instance;
		return instance;
	}

private:
	halo1_engine() = default;
	~halo1_engine() = default;

public:
	void get_game_information(halo::halo1_snapshot& snapshot);

	void set_skull_enabled(halo::Halo1Skull skull, bool enabled);

	void execute_command(const char* command);
};


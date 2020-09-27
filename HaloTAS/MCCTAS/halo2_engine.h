#pragma once
#include "halo_types.h"

class halo2_engine
{
private:
	static halo2_engine& get() {
		static halo2_engine instance;
		return instance;
	}

	halo2_engine() = default;
	~halo2_engine() = default;

private:
	static halo2::all_skulls* skull_table();

public:
	static void get_game_information(halo2::h2snapshot& snapshot);
	static void set_skull_enabled(halo2::skull skull, bool enabled);
};

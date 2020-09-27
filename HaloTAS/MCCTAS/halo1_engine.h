#pragma once
#include "halo_types.h"

class halo1_engine
{
private:
	static halo1_engine& get() {
		static halo1_engine instance;
		return instance;
	}

	halo1_engine() = default;
	~halo1_engine() = default;

private:
	static bool is_engine_active();

	static halo1::all_cheats* cheat_table();
	static halo1::all_skulls* skull_table();

public:
	static void get_game_information(halo1::h1snapshot& snapshot);

	static void set_cheat_enabled(halo1::cheat skull, bool enabled);

	static void execute_command(const char* command);

	static void set_checkpoint_flag();
	static void set_revert_flag();
};

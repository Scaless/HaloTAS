#pragma once

#include <inttypes.h>
#include <random>

struct randomizer_features {
	bool skull_anger; // fire weapons faster
	bool skull_vampire; // recharge shields with damage done
	bool skull_blind; // hide HUD
	bool skull_catch; // more grenades
	bool skull_famine; // Less ammo
	bool skull_iron; // Restart level

	bool random_level_order; 
	bool shuffle_enemies;
	bool weapon_swap;
	bool walk_on_walls;
	bool low_gravity;
	bool random_projectiles;

};

class randomizer
{
public:
	static randomizer& get() {
		static randomizer instance;
		return instance;
	}
	void operator=(randomizer const&) = delete;
	randomizer(randomizer const&) = delete;
private:
	randomizer() = default;

	std::random_device dev;
	std::mt19937 eng;

	bool active{ false };
	randomizer_features enabledFeatures{};

public:

	void set_seed(int32_t seed);

	void start_randomizer();
	void stop_randomizer();
};


#pragma once

#include <inttypes.h>
#include <random>
#include <unordered_map>

enum RANDOMIZER_FEATURE : uint32_t {
	SKULL_BLIND				= 1 << 0, // Hide HUD
	SKULL_ANGER				= 1 << 1, // Weapons fire faster
	SKULL_VAMPIRE			= 1 << 2, // Recharge shields with damage done
	SKULL_CATCH				= 1 << 3, // More Grenades
	SKULL_FAMINE			= 1 << 4, // Less Ammo
	SKULL_IRON				= 1 << 5, // Restart Level After Death
	OOPS_ALL_REVIVERS		= 1 << 6, // Flood are guaranteed to be revivers
	RANDOM_LEVEL_ORDER		= 1 << 7, // Level order will be randomized
	SHUFFLE_ENEMIES			= 1 << 8, // Enemies will be replaced with different enemy types
	WEAPON_SWAP				= 1 << 9, // Player and found weapons will be changed
	WALK_ON_WALLS			= 1 << 10, // Player/enemies can walk on walls
	GRAVITATIONAL_ANOMALY	= 1 << 11, // Change gravity more/less
	RANDOM_PROJECTILES		= 1 << 12, // Projectiles randomized (ex: AR might shoot needles)
	TIME_SCALE				= 1 << 13, // Changes time scale

};

enum class RANDOMIZER_CHANGE_TYPE {
	GLOBAL, // Randomizer will use the same options for every level
	PER_LEVEL, // Randomizer will change applied options to be unique to each level
};

struct randomizer_options {
	uint32_t feature_flags{ 0 };
	RANDOMIZER_CHANGE_TYPE change_type;
	float randomness {.5f}; // 0 to 1, 0 = nothing enabled, 1 = everything enabled
};

struct randomizer_global_rule {
	uint32_t feature_flags = 0x00000000;
};

struct randomizer_level_rule {
	uint32_t feature_flags = 0x00000000;
	float time_scale{ 1.0f };
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
	std::mt19937 eng{ dev() };

	bool enabled{ false };
	size_t seed;
	randomizer_options global_options{};
	std::vector<std::string> level_order;
	size_t currentLevelIndex{ 0 };

	// level, rules
	std::unordered_map<std::string, randomizer_level_rule> level_rules;
	randomizer_global_rule global_rule;

	randomizer_level_rule make_rule();
	void regenerate();

public:

	bool is_enabled();

	void set_seed(size_t newSeed);
	void set_options(randomizer_options newOptions);
	int get_current_level_index();

	void start();
	void stop();

	void pre_tick();
	void pre_loop();
};


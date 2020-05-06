#include "randomizer.h"
#include "halo_constants.h"
#include "halo_engine.h"
#include <algorithm>
#include <vector>

randomizer_level_rule randomizer::make_rule()
{
	randomizer_level_rule rule{};
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	
	// Turn flags on based on randomness and if they are enabled globally
	for (size_t i = 0; i < sizeof(rule.feature_flags) * 8; i++) {
		size_t flag = (1 << i);
		if (global_options.feature_flags & flag) {
			if (distribution(eng) <= global_options.randomness) {
				rule.feature_flags |= flag;
			}
		}
	}
	
	if (rule.feature_flags & RANDOMIZER_FEATURE::TIME_SCALE) {
		std::uniform_real_distribution<float> timeOffsetDistribution(.8f, 1.2f);
		rule.time_scale = timeOffsetDistribution(eng);
	}

	return rule;
}

void randomizer::regenerate()
{
	eng.seed(seed);
	currentLevelIndex = 0;

	level_order.clear();
	level_order = { "a10", "a30", "a50", "b30", "b40", "c10", "c20", "c40", "d20" };
	std::shuffle(level_order.begin(), level_order.end(), eng);
	level_order.push_back("d40"); // Always end with Maw

	// Build the rules for each level
	level_rules.clear();
	randomizer_level_rule baseRule = make_rule();
	for (auto level : level_order) {
		if (global_options.change_type == RANDOMIZER_CHANGE_TYPE::GLOBAL) {
			level_rules[level] = baseRule;
			continue;
		}
		if (global_options.change_type == RANDOMIZER_CHANGE_TYPE::PER_LEVEL) {
			auto newRule = make_rule();
			level_rules[level] = newRule;
			continue;
		}
	}

	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	randomizer_global_rule newGlobalRule{};

	if (distribution(eng) <= global_options.randomness) {
		if (global_options.feature_flags & RANDOMIZER_FEATURE::RANDOM_LEVEL_ORDER) {
			newGlobalRule.feature_flags |= RANDOMIZER_FEATURE::RANDOM_LEVEL_ORDER;
		}
	}

	global_rule = newGlobalRule;

}

bool randomizer::is_enabled()
{
	return enabled;
}

void randomizer::set_seed(size_t newSeed)
{
	seed = newSeed;
	regenerate();
}

void randomizer::set_options(randomizer_options newOptions)
{
	global_options = newOptions;
	regenerate();
}

int randomizer::get_current_level_index()
{
	return currentLevelIndex;
}

void randomizer::start()
{
	enabled = true;
	regenerate();
}

void randomizer::stop()
{
	enabled = false;
}

void randomizer::pre_tick()
{
	if (!enabled) {
		return;
	}

	std::string currentMap{ halo::addr::MAP_STRING };
	if (currentMap == "levels\\ui\\ui") {
		return;
	}

	auto& gEngine = halo_engine::get();


	

}

void randomizer::pre_loop()
{
	if (!enabled) {
		return;
	}

	std::string currentMap{ halo::addr::MAP_STRING };
	if (currentMap == "levels\\ui\\ui") {
		return;
	}

	auto& gEngine = halo_engine::get();

	std::string shortMapName = currentMap.substr(11, 3);
	auto tick = *halo::addr::SIMULATION_TICK;

	// Global rules
	if (global_rule.feature_flags & RANDOMIZER_FEATURE::RANDOM_LEVEL_ORDER) {

		if (tick == 1) {

			auto randomizerLevel = level_order[currentLevelIndex];

			if (randomizerLevel != shortMapName) {
				std::string command = "map_name " + randomizerLevel;
				gEngine.execute_command(command.c_str());
			}
		}

		if (*halo::addr::GAME_WON) {
			currentLevelIndex++;
			if (currentLevelIndex > 9) {
				currentLevelIndex = 0;
			}
		}
	}

	// Map rules
	if (level_rules.count(shortMapName)) {
		auto rule = level_rules[shortMapName];

		if (rule.feature_flags & RANDOMIZER_FEATURE::SKULL_BLIND) {
			*halo::addr::HUD_ENABLED = 0;
		}

		if (rule.feature_flags & RANDOMIZER_FEATURE::TIME_SCALE) {
			*halo::addr::GAME_SPEED = rule.time_scale;
		}

		if (rule.feature_flags & RANDOMIZER_FEATURE::OOPS_ALL_REVIVERS) {
			engine_snapshot snapshot = {};
			gEngine.get_snapshot(snapshot);

			// Find objects that are flood and set their reviver flag
			for (GameObject* v : snapshot.gameObjects) {
				const uint32_t HUMAN_FLOOD_TAG_ID = 5792;
				const uint32_t COVE_FLOOD_TAG_ID = 6024;

				if (v->tag_id == HUMAN_FLOOD_TAG_ID || v->tag_id == COVE_FLOOD_TAG_ID) {
					v->unit_flags |= (uint32_t)UNIT_FLAGS::IS_FLOOD_REVIVER;
				}
			}
		}
	}

}

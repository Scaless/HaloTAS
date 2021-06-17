#pragma once
#include "pch.h"

namespace halo1 {

	// Used to interop with the GUI program
	enum class cheat : int32_t {
		CHEAT_DEATHLESS_PLAYER,
		CHEAT_BUMP_POSESSION,
		CHEAT_SUPER_JUMP,
		CHEAT_REFLECT_DAMAGE_HITS,
		CHEAT_MEDUSA,
		CHEAT_ONE_SHOT_KILL,
		CHEAT_BOTTOMLESS_CLIP,

		SKULL_ANGER,
		SKULL_BLIND,
		SKULL_BLACK_EYE,
		SKULL_CATCH,
		SKULL_EYE_PATCH,
		SKULL_FAMINE,
		SKULL_FOG,
		SKULL_FOREIGN,
		SKULL_IRON,
		SKULL_MYTHIC,
		SKULL_RECESSION,
		SKULL_THATS_JUST_WRONG,
		SKULL_THUNDERSTORM,
		SKULL_TOUGH_LUCK,
		SKULL_BANDANA,
		SKULL_BOOM,
		SKULL_GHOST,
		SKULL_GRUNT_BIRTHDAY_PARTY,
		SKULL_GRUNT_FUNERAL,
		SKULL_MALFUNCTION,
		SKULL_PINATA,
		SKULL_SPUTNIK,
		SKULL_BOOTS_OFF_THE_GROUND,

		COUNT
	};

	struct all_cheats {
		bool cheat_deathless_player;
		bool cheat_jetpack;
		bool cheat_infinite_ammo; // Forcibly set to 0 every frame (not tick)
		bool cheat_bump_possession;
		bool cheat_super_jump;
		bool cheat_reflexive_damage_effects;
		bool cheat_medusa;
		bool cheat_omnipotent; // one shot kill
		bool padding_2; // ???
		bool cheat_bottomless_clip;
	};

	struct all_skulls {
		bool padding_1;
		bool skull_iron;
		bool skull_fog;
		bool skull_mythic;
		bool skull_famine;
		bool skull_grunt_funeral;
		bool skull_foreign;
		bool skull_eye_patch;
		bool skull_recession;
		bool skull_malfunction;
		bool skull_black_eye;
		bool skull_grunt_birthday_party;
		bool skull_pinata;
		bool padding_4;
		bool skull_bandana;
		bool skull_boom;
		bool padding_5;
		bool skull_blind;
		bool skull_ghost;
		bool padding_6;
		bool skull_catch;
		bool skull_sputnik;
		bool skull_anger;
		bool skull_thunderstorm;
		bool skull_thats_just_wrong;
		bool padding_7[5];
		bool skull_tough_luck;
		bool skull_boots_off_the_ground;
	};

	struct h1snapshot {
		bool skulls[to_underlying(cheat::COUNT)];
	};

	struct control_flags {
		bool level_restart;
		const bool game_freeze_dont_use;
		bool revert;
		bool padding1;
		bool save_safe;
		bool padding2[2];
		bool save_unsafe;
		bool padding15[15];
		bool game_won;
		bool player_dead;
		bool unk1;
		bool core_save;
		bool core_load;
		bool padding22[22];
		bool game_paused;
		bool padding6[6];
		int16_t death_counter_90;
	};
}

namespace halo2 {
	enum class skull : int32_t {
		// Scoring
		ANGER,
		ASSASSINS,
		BLACK_EYE,
		BLIND,
		CATCH,
		EYE_PATCH,
		FAMINE,
		FOG,
		IRON,
		JACKED,
		MASTERBLASTER,
		MYTHIC,
		RECESSION,
		SO_ANGRY,
		STREAKING,
		SWARM,
		THATS_JUST_WRONG,
		THEY_COME_BACK,
		THUNDERSTORM,

		// Non-scoring
		BANDANNA,
		BONDED_PAIR,
		BOOM,
		ENVY,
		FEATHER,
		GHOST,
		GRUNT_BIRTHDAY_PARTY,
		GRUNT_FUNERAL,
		IWHBYD,
		MALFUNCTION,
		PINATA,
		PROPHET_BIRTHDAY_PARTY,
		SCARAB,
		SPUTNIK,

		COUNT
	};

	struct all_skulls {
		bool skull_envy;
		bool skull_grunt_birthday_party;
		bool skull_assassins;
		bool skull_thunderstorm;
		bool skull_famine;
		bool skull_iwhbyd;
		bool skull_blind;
		bool skull_ghost;
		bool skull_black_eye;
		bool skull_catch;
		bool skull_sputnik;
		bool skull_iron;
		bool skull_mythic;
		bool skull_anger;
		bool skull_thats_just_wrong;
		bool skull_bandanna;
		bool skull_boom;
		bool skull_eye_patch;
		bool skull_fog;
		bool padding;
		bool skull_grunt_funeral;
		bool skull_pinata;
		bool skull_recession;
		bool skull_malfunction;
		bool skull_streaking;
		bool skull_jacked;
		bool skull_scarab;
		bool skull_swarm;
		bool skull_feather;
		bool skull_bonded_pair;
		bool skull_masterblaster;
		bool skull_so_angry;
		bool skull_prophet_birthday_party;
		bool skull_they_come_back;
	};

	struct h2snapshot {
		bool skulls[to_underlying(skull::COUNT)];
	};
}

#pragma once
#include "pch.h"

namespace halo {

	struct halo1_snapshot {
		bool skulls[22];
	};

	// Used to interop with the GUI program
	enum class Halo1Skull : int32_t {
		ANGER,
		BLIND,
		BLACK_EYE,
		CATCH,
		EYE_PATCH,
		FAMINE,
		FOG,
		FOREIGN,
		IRON,
		MYTHIC,
		RECESSION,
		THATS_JUST_WRONG,
		THUNDERSTORM,
		TOUGH_LUCK,
		BANDANA,
		BOOM,
		GHOST,
		GRUNT_BIRTHDAY_PARTY,
		GRUNT_FUNERAL,
		MALFUNCTION,
		PINATA,
		SPUTNIK
	};

	inline extern size_t HALO1_SKULL_IRON_OFFSET = 0x117C7C1;
	inline extern size_t HALO1_SKULL_FOG_OFFSET = 0x117C7C2;
	inline extern size_t HALO1_SKULL_MYTHIC_OFFSET = 0x117C7C3;
	inline extern size_t HALO1_SKULL_FAMINE_OFFSET = 0x117C7C4;
	inline extern size_t HALO1_SKULL_GRUNT_FUNERAL_OFFSET = 0x117C7C5;
	inline extern size_t HALO1_SKULL_FOREIGN_OFFSET = 0x117C7C6;
	inline extern size_t HALO1_SKULL_EYE_PATCH_OFFSET = 0x117C7C7;
	inline extern size_t HALO1_SKULL_RECESSION_OFFSET = 0x117C7C8;
	inline extern size_t HALO1_SKULL_MALFUNCTION_OFFSET = 0x117C7C9;
	inline extern size_t HALO1_SKULL_BLACK_EYE_OFFSET = 0x117C7CA;
	inline extern size_t HALO1_SKULL_GRUNT_BIRTHDAY_PARTY_OFFSET = 0x117C7CB;
	inline extern size_t HALO1_SKULL_PINATA_OFFSET = 0x117C7CC;
	// Padding x 1
	inline extern size_t HALO1_SKULL_BANDANA_OFFSET = 0x117C7CE;
	inline extern size_t HALO1_SKULL_BOOM_OFFSET = 0x117C7CF;
	inline extern size_t HALO1_SKULL_BLIND_OFFSET = 0x117C7D1;
	inline extern size_t HALO1_SKULL_GHOST_OFFSET = 0x117C7D2;
	// Padding x 1
	inline extern size_t HALO1_SKULL_CATCH_OFFSET = 0x117C7D4;
	inline extern size_t HALO1_SKULL_SPUTNIK_OFFSET = 0x117C7D5;
	inline extern size_t HALO1_SKULL_ANGER_OFFSET = 0x117C7D6;
	inline extern size_t HALO1_SKULL_THUNDERSTORM_OFFSET = 0x117C7D7;
	inline extern size_t HALO1_SKULL_THATS_JUST_WRONG_OFFSET = 0x117C7D8;
	// Padding x 5
	inline extern size_t HALO1_SKULL_TOUGH_LUCK_OFFSET = 0x117C7DE;

}

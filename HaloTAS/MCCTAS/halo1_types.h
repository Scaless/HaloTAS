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

	inline extern bool* HALO1_SKULL_IRON = reinterpret_cast<bool*>(0x18117C7C1);
	inline extern bool* HALO1_SKULL_FOG = reinterpret_cast<bool*>(0x18117C7C2);
	inline extern bool* HALO1_SKULL_MYTHIC = reinterpret_cast<bool*>(0x18117C7C3);
	inline extern bool* HALO1_SKULL_FAMINE = reinterpret_cast<bool*>(0x18117C7C4);
	inline extern bool* HALO1_SKULL_GRUNT_FUNERAL = reinterpret_cast<bool*>(0x18117C7C5);
	inline extern bool* HALO1_SKULL_FOREIGN = reinterpret_cast<bool*>(0x18117C7C6);
	inline extern bool* HALO1_SKULL_EYE_PATCH = reinterpret_cast<bool*>(0x18117C7C7);
	inline extern bool* HALO1_SKULL_RECESSION = reinterpret_cast<bool*>(0x18117C7C8);
	inline extern bool* HALO1_SKULL_MALFUNCTION = reinterpret_cast<bool*>(0x18117C7C9);
	inline extern bool* HALO1_SKULL_BLACK_EYE = reinterpret_cast<bool*>(0x18117C7CA);
	inline extern bool* HALO1_SKULL_GRUNT_BIRTHDAY_PARTY = reinterpret_cast<bool*>(0x18117C7CB);
	inline extern bool* HALO1_SKULL_PINATA = reinterpret_cast<bool*>(0x18117C7CC);
	// Padding x 1
	inline extern bool* HALO1_SKULL_BANDANA = reinterpret_cast<bool*>(0x18117C7CE);
	inline extern bool* HALO1_SKULL_BOOM = reinterpret_cast<bool*>(0x18117C7CF);
	inline extern bool* HALO1_SKULL_BLIND = reinterpret_cast<bool*>(0x18117C7D1);
	inline extern bool* HALO1_SKULL_GHOST = reinterpret_cast<bool*>(0x18117C7D2);
	// Padding x 1
	inline extern bool* HALO1_SKULL_CATCH = reinterpret_cast<bool*>(0x18117C7D4);
	inline extern bool* HALO1_SKULL_SPUTNIK = reinterpret_cast<bool*>(0x18117C7D5);
	inline extern bool* HALO1_SKULL_ANGER = reinterpret_cast<bool*>(0x18117C7D6);
	inline extern bool* HALO1_SKULL_THUNDERSTORM = reinterpret_cast<bool*>(0x18117C7D7);
	inline extern bool* HALO1_SKULL_THATS_JUST_WRONG = reinterpret_cast<bool*>(0x18117C7D8);
	// Padding x 5
	inline extern bool* HALO1_SKULL_TOUGH_LUCK = reinterpret_cast<bool*>(0x18117C7DE);

}

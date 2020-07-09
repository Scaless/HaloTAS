#include "halo1_engine.h"
#include "windows_utilities.h"
#include "dll_cache.h"

using namespace halo;

void halo1_engine::get_game_information(halo::halo1_snapshot& snapshot)
{
	auto H1DLL = dll_cache::get_info(HALO1_DLL_WSTR);
	if (!H1DLL.has_value()) {
		return;
	}

	auto H1Base = H1DLL.value();

	snapshot.skulls[to_underlying(Halo1Skull::ANGER)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_ANGER_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::BLIND)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_BLIND_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::BLACK_EYE)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_BLACK_EYE_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::CATCH)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_CATCH_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::EYE_PATCH)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_EYE_PATCH_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::FAMINE)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_FAMINE_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::FOG)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_FOG_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::FOREIGN)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_FOREIGN_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::IRON)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_IRON_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::MYTHIC)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_MYTHIC_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::RECESSION)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_RECESSION_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::THATS_JUST_WRONG)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_THATS_JUST_WRONG_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::THUNDERSTORM)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_THUNDERSTORM_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::TOUGH_LUCK)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_TOUGH_LUCK_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::BANDANA)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_BANDANA_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::BOOM)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_BOOM_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::GHOST)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_GHOST_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::GRUNT_BIRTHDAY_PARTY)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_GRUNT_BIRTHDAY_PARTY_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::GRUNT_FUNERAL)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_GRUNT_FUNERAL_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::MALFUNCTION)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_MALFUNCTION_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::PINATA)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_PINATA_OFFSET);
	snapshot.skulls[to_underlying(Halo1Skull::SPUTNIK)] = *(bool*)((uint8_t*)H1Base+HALO1_SKULL_SPUTNIK_OFFSET);
}

void halo1_engine::set_skull_enabled(halo::Halo1Skull skull, bool enabled)
{
	auto H1DLL = dll_cache::get_info(HALO1_DLL_WSTR);
	if (!H1DLL.has_value()) {
		return;
	}

	auto H1Base = H1DLL.value();

	switch (skull) {
	case Halo1Skull::ANGER:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_ANGER_OFFSET) = enabled;
		break;
	case Halo1Skull::BLIND:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_BLIND_OFFSET) = enabled;
		break;
	case Halo1Skull::BLACK_EYE:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_BLACK_EYE_OFFSET) = enabled;
		break;
	case Halo1Skull::CATCH:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_CATCH_OFFSET) = enabled;
		break;
	case Halo1Skull::EYE_PATCH:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_EYE_PATCH_OFFSET) = enabled;
		break;
	case Halo1Skull::FAMINE:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_FAMINE_OFFSET) = enabled;
		break;
	case Halo1Skull::FOG:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_FOG_OFFSET) = enabled;
		break;
	case Halo1Skull::FOREIGN:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_FOREIGN_OFFSET) = enabled;
		break;
	case Halo1Skull::IRON:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_IRON_OFFSET) = enabled;
		break;
	case Halo1Skull::MYTHIC:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_MYTHIC_OFFSET) = enabled;
		break;
	case Halo1Skull::RECESSION:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_RECESSION_OFFSET) = enabled;
		break;
	case Halo1Skull::THATS_JUST_WRONG:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_THATS_JUST_WRONG_OFFSET) = enabled;
		break;
	case Halo1Skull::THUNDERSTORM:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_THUNDERSTORM_OFFSET) = enabled;
		break;
	case Halo1Skull::TOUGH_LUCK:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_TOUGH_LUCK_OFFSET) = enabled;
		break;
	case Halo1Skull::BANDANA:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_BANDANA_OFFSET) = enabled;
		break;
	case Halo1Skull::BOOM:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_BOOM_OFFSET) = enabled;
		break;
	case Halo1Skull::GHOST:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_GHOST_OFFSET) = enabled;
		break;
	case Halo1Skull::GRUNT_BIRTHDAY_PARTY:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_GRUNT_BIRTHDAY_PARTY_OFFSET) = enabled;
		break;
	case Halo1Skull::GRUNT_FUNERAL:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_GRUNT_FUNERAL_OFFSET) = enabled;
		break;
	case Halo1Skull::MALFUNCTION:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_MALFUNCTION_OFFSET) = enabled;
		break;
	case Halo1Skull::PINATA:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_PINATA_OFFSET) = enabled;
		break;
	case Halo1Skull::SPUTNIK:
		*(bool*)((uint8_t*)H1Base+HALO1_SKULL_SPUTNIK_OFFSET) = enabled;
		break;
	}
}

typedef char __fastcall ExecuteCommand(const char* src, uint16_t a2);
void halo1_engine::execute_command(const char* command)
{
	auto H1DLL = dll_cache::get_info(HALO1_DLL_WSTR);
	
	if (H1DLL.has_value()) {
		ExecuteCommand* Exec = (ExecuteCommand*)((uint8_t*)H1DLL.value() + 0x7ED5A0);
		Exec(command, 0);
	}
}

#include "halo1_engine.h"

using namespace halo;

void halo1_engine::get_game_information(halo::halo1_snapshot& snapshot)
{
	snapshot.skulls[to_underlying(Halo1Skull::ANGER)] = *HALO1_SKULL_ANGER;
	snapshot.skulls[to_underlying(Halo1Skull::BLIND)] = *HALO1_SKULL_BLIND;
	snapshot.skulls[to_underlying(Halo1Skull::BLACK_EYE)] = *HALO1_SKULL_BLACK_EYE;
	snapshot.skulls[to_underlying(Halo1Skull::CATCH)] = *HALO1_SKULL_CATCH;
	snapshot.skulls[to_underlying(Halo1Skull::EYE_PATCH)] = *HALO1_SKULL_EYE_PATCH;
	snapshot.skulls[to_underlying(Halo1Skull::FAMINE)] = *HALO1_SKULL_FAMINE;
	snapshot.skulls[to_underlying(Halo1Skull::FOG)] = *HALO1_SKULL_FOG;
	snapshot.skulls[to_underlying(Halo1Skull::FOREIGN)] = *HALO1_SKULL_FOREIGN;
	snapshot.skulls[to_underlying(Halo1Skull::IRON)] = *HALO1_SKULL_IRON;
	snapshot.skulls[to_underlying(Halo1Skull::MYTHIC)] = *HALO1_SKULL_MYTHIC;
	snapshot.skulls[to_underlying(Halo1Skull::RECESSION)] = *HALO1_SKULL_RECESSION;
	snapshot.skulls[to_underlying(Halo1Skull::THATS_JUST_WRONG)] = *HALO1_SKULL_THATS_JUST_WRONG;
	snapshot.skulls[to_underlying(Halo1Skull::THUNDERSTORM)] = *HALO1_SKULL_THUNDERSTORM;
	snapshot.skulls[to_underlying(Halo1Skull::TOUGH_LUCK)] = *HALO1_SKULL_TOUGH_LUCK;
	snapshot.skulls[to_underlying(Halo1Skull::BANDANA)] = *HALO1_SKULL_BANDANA;
	snapshot.skulls[to_underlying(Halo1Skull::BOOM)] = *HALO1_SKULL_BOOM;
	snapshot.skulls[to_underlying(Halo1Skull::GHOST)] = *HALO1_SKULL_GHOST;
	snapshot.skulls[to_underlying(Halo1Skull::GRUNT_BIRTHDAY_PARTY)] = *HALO1_SKULL_GRUNT_BIRTHDAY_PARTY;
	snapshot.skulls[to_underlying(Halo1Skull::GRUNT_FUNERAL)] = *HALO1_SKULL_GRUNT_FUNERAL;
	snapshot.skulls[to_underlying(Halo1Skull::MALFUNCTION)] = *HALO1_SKULL_MALFUNCTION;
	snapshot.skulls[to_underlying(Halo1Skull::PINATA)] = *HALO1_SKULL_PINATA;
	snapshot.skulls[to_underlying(Halo1Skull::SPUTNIK)] = *HALO1_SKULL_SPUTNIK;
}

void halo1_engine::set_skull_enabled(halo::Halo1Skull skull, bool enabled)
{
	switch (skull) {
	case Halo1Skull::ANGER:
		*HALO1_SKULL_ANGER = enabled;
		break;
	case Halo1Skull::BLIND:
		*HALO1_SKULL_BLIND = enabled;
		break;
	case Halo1Skull::BLACK_EYE:
		*HALO1_SKULL_BLACK_EYE = enabled;
		break;
	case Halo1Skull::CATCH:
		*HALO1_SKULL_CATCH = enabled;
		break;
	case Halo1Skull::EYE_PATCH:
		*HALO1_SKULL_EYE_PATCH = enabled;
		break;
	case Halo1Skull::FAMINE:
		*HALO1_SKULL_FAMINE = enabled;
		break;
	case Halo1Skull::FOG:
		*HALO1_SKULL_FOG = enabled;
		break;
	case Halo1Skull::FOREIGN:
		*HALO1_SKULL_FOREIGN = enabled;
		break;
	case Halo1Skull::IRON:
		*HALO1_SKULL_IRON = enabled;
		break;
	case Halo1Skull::MYTHIC:
		*HALO1_SKULL_MYTHIC = enabled;
		break;
	case Halo1Skull::RECESSION:
		*HALO1_SKULL_RECESSION = enabled;
		break;
	case Halo1Skull::THATS_JUST_WRONG:
		*HALO1_SKULL_THATS_JUST_WRONG = enabled;
		break;
	case Halo1Skull::THUNDERSTORM:
		*HALO1_SKULL_THUNDERSTORM = enabled;
		break;
	case Halo1Skull::TOUGH_LUCK:
		*HALO1_SKULL_TOUGH_LUCK = enabled;
		break;
	case Halo1Skull::BANDANA:
		*HALO1_SKULL_BANDANA = enabled;
		break;
	case Halo1Skull::BOOM:
		*HALO1_SKULL_BOOM = enabled;
		break;
	case Halo1Skull::GHOST:
		*HALO1_SKULL_GHOST = enabled;
		break;
	case Halo1Skull::GRUNT_BIRTHDAY_PARTY:
		*HALO1_SKULL_GRUNT_BIRTHDAY_PARTY = enabled;
		break;
	case Halo1Skull::GRUNT_FUNERAL:
		*HALO1_SKULL_GRUNT_FUNERAL = enabled;
		break;
	case Halo1Skull::MALFUNCTION:
		*HALO1_SKULL_MALFUNCTION = enabled;
		break;
	case Halo1Skull::PINATA:
		*HALO1_SKULL_PINATA = enabled;
		break;
	case Halo1Skull::SPUTNIK:
		*HALO1_SKULL_SPUTNIK = enabled;
		break;
	}
}

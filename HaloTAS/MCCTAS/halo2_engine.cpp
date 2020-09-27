#include "pch.h"
#include "halo2_engine.h"
#include "signatures.h"

using namespace halo2;

halo2::all_skulls* halo2_engine::skull_table()
{
    auto skull_offset = sig_halo2_skull_table_offset.opt_value();
	if (skull_offset) {
		auto H2DLL = dll_cache::get_module_handle(HALO2_DLL_WSTR);
		if (!H2DLL.has_value()) {
			return nullptr;
		}

		auto skull_ptr = ((uint8_t*)H2DLL.value()) + *skull_offset.value();
		return (halo2::all_skulls*)skull_ptr;
	}

    return nullptr;
}

void halo2_engine::get_game_information(halo2::h2snapshot& snapshot)
{
	auto H2DLL = dll_cache::get_module_handle(HALO2_DLL_WSTR);
	if (!H2DLL.has_value()) {
		return;
	}

	auto skulls = skull_table();
	if (!skulls) {
		return;
	}

	snapshot.skulls[to_underlying(skull::ANGER)] = skulls->skull_anger;
	snapshot.skulls[to_underlying(skull::ASSASSINS)] = skulls->skull_assassins;
	snapshot.skulls[to_underlying(skull::BLACK_EYE)] = skulls->skull_black_eye;
	snapshot.skulls[to_underlying(skull::BLIND)] = skulls->skull_blind;
	snapshot.skulls[to_underlying(skull::CATCH)] = skulls->skull_catch;
	snapshot.skulls[to_underlying(skull::EYE_PATCH)] = skulls->skull_eye_patch;
	snapshot.skulls[to_underlying(skull::FAMINE)] = skulls->skull_famine;
	snapshot.skulls[to_underlying(skull::FOG)] = skulls->skull_fog;
	snapshot.skulls[to_underlying(skull::IRON)] = skulls->skull_iron;
	snapshot.skulls[to_underlying(skull::JACKED)] = skulls->skull_jacked;
	snapshot.skulls[to_underlying(skull::MASTERBLASTER)] = skulls->skull_masterblaster;
	snapshot.skulls[to_underlying(skull::MYTHIC)] = skulls->skull_mythic;
	snapshot.skulls[to_underlying(skull::RECESSION)] = skulls->skull_recession;
	snapshot.skulls[to_underlying(skull::SO_ANGRY)] = skulls->skull_so_angry;
	snapshot.skulls[to_underlying(skull::STREAKING)] = skulls->skull_streaking;
	snapshot.skulls[to_underlying(skull::SWARM)] = skulls->skull_swarm;
	snapshot.skulls[to_underlying(skull::THATS_JUST_WRONG)] = skulls->skull_thats_just_wrong;
	snapshot.skulls[to_underlying(skull::THEY_COME_BACK)] = skulls->skull_they_come_back;
	snapshot.skulls[to_underlying(skull::THUNDERSTORM)] = skulls->skull_thunderstorm;
	snapshot.skulls[to_underlying(skull::BANDANNA)] = skulls->skull_bandanna;
	snapshot.skulls[to_underlying(skull::BONDED_PAIR)] = skulls->skull_bonded_pair;
	snapshot.skulls[to_underlying(skull::BOOM)] = skulls->skull_boom;
	snapshot.skulls[to_underlying(skull::ENVY)] = skulls->skull_envy;
	snapshot.skulls[to_underlying(skull::FEATHER)] = skulls->skull_feather;
	snapshot.skulls[to_underlying(skull::GHOST)] = skulls->skull_ghost;
	snapshot.skulls[to_underlying(skull::GRUNT_BIRTHDAY_PARTY)] = skulls->skull_grunt_birthday_party;
	snapshot.skulls[to_underlying(skull::GRUNT_FUNERAL)] = skulls->skull_grunt_funeral;
	snapshot.skulls[to_underlying(skull::IWHBYD)] = skulls->skull_iwhbyd;
	snapshot.skulls[to_underlying(skull::MALFUNCTION)] = skulls->skull_malfunction;
	snapshot.skulls[to_underlying(skull::PINATA)] = skulls->skull_pinata;
	snapshot.skulls[to_underlying(skull::PROPHET_BIRTHDAY_PARTY)] = skulls->skull_prophet_birthday_party;
	snapshot.skulls[to_underlying(skull::SCARAB)] = skulls->skull_scarab;
	snapshot.skulls[to_underlying(skull::SPUTNIK)] = skulls->skull_sputnik;
}

void halo2_engine::set_skull_enabled(halo2::skull skull, bool enabled)
{
	auto H2DLL = dll_cache::get_module_handle(HALO2_DLL_WSTR);
	if (!H2DLL.has_value()) {
		return;
	}

	auto skulls = skull_table();
	if (!skulls) {
		return;
	}

	switch (skull)
	{
	case halo2::skull::ANGER:
		skulls->skull_anger = enabled;
		break;
	case halo2::skull::ASSASSINS:
		skulls->skull_assassins = enabled;
		break;
	case halo2::skull::BLACK_EYE:
		skulls->skull_black_eye = enabled;
		break;
	case halo2::skull::BLIND:
		skulls->skull_blind = enabled;
		break;
	case halo2::skull::CATCH:
		skulls->skull_catch = enabled;
		break;
	case halo2::skull::EYE_PATCH:
		skulls->skull_eye_patch = enabled;
		break;
	case halo2::skull::FAMINE:
		skulls->skull_famine = enabled;
		break;
	case halo2::skull::FOG:
		skulls->skull_fog = enabled;
		break;
	case halo2::skull::IRON:
		skulls->skull_iron = enabled;
		break;
	case halo2::skull::JACKED:
		skulls->skull_jacked = enabled;
		break;
	case halo2::skull::MASTERBLASTER:
		skulls->skull_masterblaster = enabled;
		break;
	case halo2::skull::MYTHIC:
		skulls->skull_mythic = enabled;
		break;
	case halo2::skull::RECESSION:
		skulls->skull_recession = enabled;
		break;
	case halo2::skull::SO_ANGRY:
		skulls->skull_so_angry = enabled;
		break;
	case halo2::skull::STREAKING:
		skulls->skull_streaking = enabled;
		break;
	case halo2::skull::SWARM:
		skulls->skull_swarm = enabled;
		break;
	case halo2::skull::THATS_JUST_WRONG:
		skulls->skull_thats_just_wrong = enabled;
		break;
	case halo2::skull::THEY_COME_BACK:
		skulls->skull_they_come_back = enabled;
		break;
	case halo2::skull::THUNDERSTORM:
		skulls->skull_thunderstorm = enabled;
		break;
	case halo2::skull::BANDANNA:
		skulls->skull_bandanna = enabled;
		break;
	case halo2::skull::BONDED_PAIR:
		skulls->skull_bonded_pair = enabled;
		break;
	case halo2::skull::BOOM:
		skulls->skull_boom = enabled;
		break;
	case halo2::skull::ENVY:
		skulls->skull_envy = enabled;
		break;
	case halo2::skull::FEATHER:
		skulls->skull_feather = enabled;
		break;
	case halo2::skull::GHOST:
		skulls->skull_ghost = enabled;
		break;
	case halo2::skull::GRUNT_BIRTHDAY_PARTY:
		skulls->skull_grunt_birthday_party = enabled;
		break;
	case halo2::skull::GRUNT_FUNERAL:
		skulls->skull_grunt_funeral = enabled;
		break;
	case halo2::skull::IWHBYD:
		skulls->skull_iwhbyd = enabled;
		break;
	case halo2::skull::MALFUNCTION:
		skulls->skull_malfunction = enabled;
		break;
	case halo2::skull::PINATA:
		skulls->skull_pinata = enabled;
		break;
	case halo2::skull::PROPHET_BIRTHDAY_PARTY:
		skulls->skull_prophet_birthday_party = enabled;
		break;
	case halo2::skull::SCARAB:
		skulls->skull_scarab = enabled;
		break;
	case halo2::skull::SPUTNIK:
		skulls->skull_sputnik = enabled;
		break;
	}
}

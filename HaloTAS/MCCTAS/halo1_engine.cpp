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
	halo1_cheats* cheats = (halo1_cheats*)((uint8_t*)H1Base + halo1::data::OFFSET_CHEAT_TABLE);

	snapshot.skulls[to_underlying(Halo1Cheat::CHEAT_DEATHLESS_PLAYER)] = cheats->cheat_deathless_player;
	snapshot.skulls[to_underlying(Halo1Cheat::CHEAT_BUMP_POSESSION)] = cheats->cheat_bump_possession;
	snapshot.skulls[to_underlying(Halo1Cheat::CHEAT_SUPER_JUMP)] = cheats->cheat_super_jump;
	snapshot.skulls[to_underlying(Halo1Cheat::CHEAT_REFLECT_DAMAGE_HITS)] = cheats->cheat_reflect_damage_hits;
	snapshot.skulls[to_underlying(Halo1Cheat::CHEAT_MEDUSA)] = cheats->cheat_medusa;
	snapshot.skulls[to_underlying(Halo1Cheat::CHEAT_ONE_SHOT_KILL)] = cheats->cheat_one_shot_kill;
	snapshot.skulls[to_underlying(Halo1Cheat::CHEAT_BOTTOMLESS_CLIP)] = cheats->cheat_bottomless_clip;

	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_ANGER)] = cheats->skull_anger;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_BLIND)] = cheats->skull_blind;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_BLACK_EYE)] = cheats->skull_black_eye;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_CATCH)] = cheats->skull_catch;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_EYE_PATCH)] = cheats->skull_eye_patch;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_FAMINE)] = cheats->skull_famine;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_FOG)] = cheats->skull_fog;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_FOREIGN)] = cheats->skull_foreign;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_IRON)] = cheats->skull_iron;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_MYTHIC)] = cheats->skull_mythic;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_RECESSION)] = cheats->skull_recession;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_THATS_JUST_WRONG)] = cheats->skull_thats_just_wrong;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_THUNDERSTORM)] = cheats->skull_thunderstorm;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_TOUGH_LUCK)] = cheats->skull_tough_luck;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_BANDANA)] = cheats->skull_bandana;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_BOOM)] = cheats->skull_boom;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_GHOST)] = cheats->skull_ghost;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_GRUNT_BIRTHDAY_PARTY)] = cheats->skull_grunt_birthday_party;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_GRUNT_FUNERAL)] = cheats->skull_grunt_funeral;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_MALFUNCTION)] = cheats->skull_malfunction;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_PINATA)] = cheats->skull_pinata;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_SPUTNIK)] = cheats->skull_sputnik;
	snapshot.skulls[to_underlying(Halo1Cheat::SKULL_BOOTS_OFF_THE_GROUND)] = cheats->skull_boots_off_the_ground;
}

void halo1_engine::set_cheat_enabled(halo::Halo1Cheat cheat, bool enabled)
{
	auto H1DLL = dll_cache::get_info(HALO1_DLL_WSTR);
	if (!H1DLL.has_value()) {
		return;
	}

	auto H1Base = H1DLL.value();
	halo1_cheats* cheats = (halo1_cheats*)((uint8_t*)H1Base + halo1::data::OFFSET_CHEAT_TABLE);

	switch (cheat) {
		// Cheats
		///////////////////////////////////////////////////////////////////////////////
	case Halo1Cheat::CHEAT_DEATHLESS_PLAYER:
		cheats->cheat_deathless_player = enabled;
		break;
	case Halo1Cheat::CHEAT_BUMP_POSESSION:
		cheats->cheat_bump_possession = enabled;
		break;
	case Halo1Cheat::CHEAT_SUPER_JUMP:
		cheats->cheat_super_jump = enabled;
		break;
	case Halo1Cheat::CHEAT_REFLECT_DAMAGE_HITS:
		cheats->cheat_reflect_damage_hits = enabled;
		break;
	case Halo1Cheat::CHEAT_MEDUSA:
		cheats->cheat_medusa = enabled;
		break;
	case Halo1Cheat::CHEAT_ONE_SHOT_KILL:
		cheats->cheat_one_shot_kill = enabled;
		break;
	case Halo1Cheat::CHEAT_BOTTOMLESS_CLIP:
		cheats->cheat_bottomless_clip = enabled;
		break;
		// Skulls
		///////////////////////////////////////////////////////////////////////////////
	case Halo1Cheat::SKULL_ANGER:
		cheats->skull_anger = enabled;
		break;
	case Halo1Cheat::SKULL_BLIND:
		cheats->skull_blind = enabled;
		break;
	case Halo1Cheat::SKULL_BLACK_EYE:
		cheats->skull_black_eye = enabled;
		break;
	case Halo1Cheat::SKULL_CATCH:
		cheats->skull_catch = enabled;
		break;
	case Halo1Cheat::SKULL_EYE_PATCH:
		cheats->skull_eye_patch = enabled;
		break;
	case Halo1Cheat::SKULL_FAMINE:
		cheats->skull_famine = enabled;
		break;
	case Halo1Cheat::SKULL_FOG:
		cheats->skull_fog = enabled;
		break;
	case Halo1Cheat::SKULL_FOREIGN:
		cheats->skull_foreign = enabled;
		break;
	case Halo1Cheat::SKULL_IRON:
		cheats->skull_iron = enabled;
		break;
	case Halo1Cheat::SKULL_MYTHIC:
		cheats->skull_mythic = enabled;
		break;
	case Halo1Cheat::SKULL_RECESSION:
		cheats->skull_recession = enabled;
		break;
	case Halo1Cheat::SKULL_THATS_JUST_WRONG:
		cheats->skull_thats_just_wrong = enabled;
		break;
	case Halo1Cheat::SKULL_THUNDERSTORM:
		cheats->skull_thunderstorm = enabled;
		break;
	case Halo1Cheat::SKULL_TOUGH_LUCK:
		cheats->skull_tough_luck = enabled;
		break;
	case Halo1Cheat::SKULL_BANDANA:
		cheats->skull_bandana = enabled;
		break;
	case Halo1Cheat::SKULL_BOOM:
		cheats->skull_boom = enabled;
		break;
	case Halo1Cheat::SKULL_GHOST:
		cheats->skull_ghost = enabled;
		break;
	case Halo1Cheat::SKULL_GRUNT_BIRTHDAY_PARTY:
		cheats->skull_grunt_birthday_party = enabled;
		break;
	case Halo1Cheat::SKULL_GRUNT_FUNERAL:
		cheats->skull_grunt_funeral = enabled;
		break;
	case Halo1Cheat::SKULL_MALFUNCTION:
		cheats->skull_malfunction = enabled;
		break;
	case Halo1Cheat::SKULL_PINATA:
		cheats->skull_pinata = enabled;
		break;
	case Halo1Cheat::SKULL_SPUTNIK:
		cheats->skull_sputnik = enabled;
		break;
	case Halo1Cheat::SKULL_BOOTS_OFF_THE_GROUND:
		cheats->skull_boots_off_the_ground = enabled;
		break;
	}
}

typedef char __fastcall ExecuteCommand(const char* src, uint16_t a2);
void halo1_engine::execute_command(const char* command)
{
	auto H1DLL = dll_cache::get_info(HALO1_DLL_WSTR);
	if (H1DLL.has_value()) {

		// Check that runtime data is accessible, if not we're probably on the main menu
		auto data_ptr = (void**)((uint8_t*)H1DLL.value() + halo1::data::OFFSET_TICK_BASE);
		if (*data_ptr != nullptr) {
			ExecuteCommand* Exec = (ExecuteCommand*)((uint8_t*)H1DLL.value() + halo1::function::OFFSET_H1_EXECUTE_COMMAND);
			Exec(command, 0);
		}
	}
}

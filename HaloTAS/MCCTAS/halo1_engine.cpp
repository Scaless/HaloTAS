#include "pch.h"
#include "halo1_engine.h"
#include "windows_utilities.h"
#include "dll_cache.h"
#include "signatures.h"
#include "tas_hooks.h"

using namespace halo1;

bool halo1_engine::is_engine_active()
{
	auto H1DLL = dll_cache::get_module_handle(HALO1_DLL_WSTR);

	if (!H1DLL.has_value()) {
		return false;
	}

	// Check that runtime data is accessible, if not we're probably not running
	// TODO-SCALES: Make this more reliable
	auto tick_ptr = value_ptr<int32_t>(H1DLL.value(), halo1::data::OFFSET_TICK_BASE, { 0xC });
	if (tick_ptr != nullptr) {
		return true;
	}

	return false;
}

halo1::all_cheats* halo1_engine::cheat_table()
{
	auto cheat_instruction = sig_halo1_cheat_table_relative.opt_value();
	if (cheat_instruction) {
		return relative_offset_to_value<halo1::all_cheats>((int64_t)cheat_instruction.value(), *cheat_instruction.value(), sizeof(*cheat_instruction.value()));
	}

	return nullptr;
}

halo1::all_skulls* halo1_engine::skull_table()
{
	auto skull_offset = sig_halo1_skull_table_offset.opt_value();
	if (skull_offset) {
		auto H1DLL = dll_cache::get_module_handle(HALO1_DLL_WSTR);
		if (!H1DLL.has_value()) {
			return nullptr;
		}

		auto skull_ptr = ((uint8_t*)H1DLL.value()) + *skull_offset.value();
		return (all_skulls*)skull_ptr;
	}

	return nullptr;
}

void halo1_engine::get_game_information(h1snapshot& snapshot)
{
	auto H1DLL = dll_cache::get_module_handle(HALO1_DLL_WSTR);
	if (!H1DLL.has_value()) {
		return;
	}

	auto cheats = cheat_table();
	if (!cheats) {
		return;
	}

	auto skulls = skull_table();
	if (!skulls) {
		return;
	}

	snapshot.skulls[to_underlying(cheat::CHEAT_DEATHLESS_PLAYER)] = cheats->cheat_deathless_player;
	snapshot.skulls[to_underlying(cheat::CHEAT_BUMP_POSESSION)] = cheats->cheat_bump_possession;
	snapshot.skulls[to_underlying(cheat::CHEAT_SUPER_JUMP)] = cheats->cheat_super_jump;
	snapshot.skulls[to_underlying(cheat::CHEAT_REFLECT_DAMAGE_HITS)] = cheats->cheat_reflect_damage_hits;
	snapshot.skulls[to_underlying(cheat::CHEAT_MEDUSA)] = cheats->cheat_medusa;
	snapshot.skulls[to_underlying(cheat::CHEAT_ONE_SHOT_KILL)] = cheats->cheat_one_shot_kill;
	snapshot.skulls[to_underlying(cheat::CHEAT_BOTTOMLESS_CLIP)] = cheats->cheat_bottomless_clip;

	snapshot.skulls[to_underlying(cheat::SKULL_ANGER)] = skulls->skull_anger;
	snapshot.skulls[to_underlying(cheat::SKULL_BLIND)] = skulls->skull_blind;
	snapshot.skulls[to_underlying(cheat::SKULL_BLACK_EYE)] = skulls->skull_black_eye;
	snapshot.skulls[to_underlying(cheat::SKULL_CATCH)] = skulls->skull_catch;
	snapshot.skulls[to_underlying(cheat::SKULL_EYE_PATCH)] = skulls->skull_eye_patch;
	snapshot.skulls[to_underlying(cheat::SKULL_FAMINE)] = skulls->skull_famine;
	snapshot.skulls[to_underlying(cheat::SKULL_FOG)] = skulls->skull_fog;
	snapshot.skulls[to_underlying(cheat::SKULL_FOREIGN)] = skulls->skull_foreign;
	snapshot.skulls[to_underlying(cheat::SKULL_IRON)] = skulls->skull_iron;
	snapshot.skulls[to_underlying(cheat::SKULL_MYTHIC)] = skulls->skull_mythic;
	snapshot.skulls[to_underlying(cheat::SKULL_RECESSION)] = skulls->skull_recession;
	snapshot.skulls[to_underlying(cheat::SKULL_THATS_JUST_WRONG)] = skulls->skull_thats_just_wrong;
	snapshot.skulls[to_underlying(cheat::SKULL_THUNDERSTORM)] = skulls->skull_thunderstorm;
	snapshot.skulls[to_underlying(cheat::SKULL_TOUGH_LUCK)] = skulls->skull_tough_luck;
	snapshot.skulls[to_underlying(cheat::SKULL_BANDANA)] = skulls->skull_bandana;
	snapshot.skulls[to_underlying(cheat::SKULL_BOOM)] = skulls->skull_boom;
	snapshot.skulls[to_underlying(cheat::SKULL_GHOST)] = skulls->skull_ghost;
	snapshot.skulls[to_underlying(cheat::SKULL_GRUNT_BIRTHDAY_PARTY)] = skulls->skull_grunt_birthday_party;
	snapshot.skulls[to_underlying(cheat::SKULL_GRUNT_FUNERAL)] = skulls->skull_grunt_funeral;
	snapshot.skulls[to_underlying(cheat::SKULL_MALFUNCTION)] = skulls->skull_malfunction;
	snapshot.skulls[to_underlying(cheat::SKULL_PINATA)] = skulls->skull_pinata;
	snapshot.skulls[to_underlying(cheat::SKULL_SPUTNIK)] = skulls->skull_sputnik;
	snapshot.skulls[to_underlying(cheat::SKULL_BOOTS_OFF_THE_GROUND)] = skulls->skull_boots_off_the_ground;
}

void halo1_engine::set_cheat_enabled(halo1::cheat cheat, bool enabled)
{
	auto H1DLL = dll_cache::get_module_handle(HALO1_DLL_WSTR);
	if (!H1DLL.has_value()) {
		return;
	}

	auto cheats = cheat_table();
	if (!cheats) {
		return;
	}

	auto skulls = skull_table();
	if (!skulls) {
		return;
	}

	switch (cheat) {
		// Cheats
		///////////////////////////////////////////////////////////////////////////////
	case cheat::CHEAT_DEATHLESS_PLAYER:
		cheats->cheat_deathless_player = enabled;
		break;
	case cheat::CHEAT_BUMP_POSESSION:
		cheats->cheat_bump_possession = enabled;
		break;
	case cheat::CHEAT_SUPER_JUMP:
		cheats->cheat_super_jump = enabled;
		break;
	case cheat::CHEAT_REFLECT_DAMAGE_HITS:
		cheats->cheat_reflect_damage_hits = enabled;
		break;
	case cheat::CHEAT_MEDUSA:
		cheats->cheat_medusa = enabled;
		break;
	case cheat::CHEAT_ONE_SHOT_KILL:
		cheats->cheat_one_shot_kill = enabled;
		break;
	case cheat::CHEAT_BOTTOMLESS_CLIP:
		cheats->cheat_bottomless_clip = enabled;
		break;
		// Skulls
		///////////////////////////////////////////////////////////////////////////////
	case cheat::SKULL_ANGER:
		skulls->skull_anger = enabled;
		break;
	case cheat::SKULL_BLIND:
		skulls->skull_blind = enabled;
		break;
	case cheat::SKULL_BLACK_EYE:
		skulls->skull_black_eye = enabled;
		break;
	case cheat::SKULL_CATCH:
		skulls->skull_catch = enabled;
		break;
	case cheat::SKULL_EYE_PATCH:
		skulls->skull_eye_patch = enabled;
		break;
	case cheat::SKULL_FAMINE:
		skulls->skull_famine = enabled;
		break;
	case cheat::SKULL_FOG:
		skulls->skull_fog = enabled;
		break;
	case cheat::SKULL_FOREIGN:
		skulls->skull_foreign = enabled;
		break;
	case cheat::SKULL_IRON:
		skulls->skull_iron = enabled;
		break;
	case cheat::SKULL_MYTHIC:
		skulls->skull_mythic = enabled;
		break;
	case cheat::SKULL_RECESSION:
		skulls->skull_recession = enabled;
		break;
	case cheat::SKULL_THATS_JUST_WRONG:
		skulls->skull_thats_just_wrong = enabled;
		break;
	case cheat::SKULL_THUNDERSTORM:
		skulls->skull_thunderstorm = enabled;
		break;
	case cheat::SKULL_TOUGH_LUCK:
		skulls->skull_tough_luck = enabled;
		break;
	case cheat::SKULL_BANDANA:
		skulls->skull_bandana = enabled;
		break;
	case cheat::SKULL_BOOM:
		skulls->skull_boom = enabled;
		break;
	case cheat::SKULL_GHOST:
		skulls->skull_ghost = enabled;
		break;
	case cheat::SKULL_GRUNT_BIRTHDAY_PARTY:
		skulls->skull_grunt_birthday_party = enabled;
		break;
	case cheat::SKULL_GRUNT_FUNERAL:
		skulls->skull_grunt_funeral = enabled;
		break;
	case cheat::SKULL_MALFUNCTION:
		skulls->skull_malfunction = enabled;
		break;
	case cheat::SKULL_PINATA:
		skulls->skull_pinata = enabled;
		break;
	case cheat::SKULL_SPUTNIK:
		skulls->skull_sputnik = enabled;
		break;
	case cheat::SKULL_BOOTS_OFF_THE_GROUND:
		skulls->skull_boots_off_the_ground = enabled;
		break;
	}
}

typedef char __fastcall ExecuteCommand(const char* src, uint16_t a2);
void halo1_engine::execute_command(const char* command)
{
	if (!is_engine_active())
		return;

	/*auto H1DLL = dll_cache::get_module_handle(HALO1_DLL_WSTR);
	if (H1DLL.has_value()) {
		auto Exec = value_ptr<ExecuteCommand>(H1DLL.value(), halo1::function::OFFSET_H1_EXECUTE_COMMAND);
		Exec(command, 0);
	}*/
	tas_hooks::execute_halo1_command(command);
}

void halo1_engine::set_checkpoint_flag()
{
	if (!is_engine_active())
		return;

	auto H1DLL = dll_cache::get_module_handle(HALO1_DLL_WSTR);
	if (H1DLL.has_value()) {
		auto cf = value_ptr<control_flags>(H1DLL.value(), halo1::data::OFFSET_CONTROL_FLAGS);
		cf->save_unsafe = true;
	}
}

void halo1_engine::set_revert_flag()
{
	if (!is_engine_active())
		return;

	auto H1DLL = dll_cache::get_module_handle(HALO1_DLL_WSTR);
	if (H1DLL.has_value()) {
		auto cf = value_ptr<control_flags>(H1DLL.value(), halo1::data::OFFSET_CONTROL_FLAGS);
		cf->revert = true;
		cf->player_dead = false;
		cf->death_counter_90 = 0;
	}
}

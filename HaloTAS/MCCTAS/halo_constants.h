#pragma once

#include <cinttypes>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "tas_utilities.h"

namespace {
	const std::wstring HALO1_DLL_WSTR = L"halo1.dll";
	const std::wstring HALO2_DLL_WSTR = L"halo2.dll";
	const std::wstring HALO3_DLL_WSTR = L"halo3.dll";
	const std::wstring HALOODST_DLL_WSTR = L"halo3odst.dll";
	const std::wstring HALOREACH_DLL_WSTR = L"haloreach.dll";
	const std::wstring HALO4_DLL_WSTR = L"halo4.dll";
}

namespace mcc {
	namespace function {
		inline const uint64_t OFFSET_MCCGETINPUT = 0x18E3714;
	}
}

namespace halo3 {
	namespace function {
		inline const uint64_t OFFSET_H3_TICK = 0x29F1B0;
	}
	namespace data {
		inline const uint64_t OFFSET_TICK_TLSINDEX = 0x9F219C;
		inline const uint64_t OFFSET_RNG = 0xE328C0;
	}
}

namespace halo2 {
	namespace function {
		inline const uint64_t OFFSET_H2_TICK = 0x6CC540;
		inline const uint64_t OFFSET_H2_TICK_LOOP = 0x6CCA10;
	}
	namespace data {
		inline const uint64_t OFFSET_TICK_BASE = 0x13B7F00;
		inline const uint64_t OFFSET_RNG = 0xD76720;
	}
}

namespace halo1 {

	namespace function {
		// TODO-SCALES - Get scan for this func
		inline const uint64_t OFFSET_H1_HANDLE_INPUT = 0x70EBC0;

		// SCAN: 48 8B C4 53 57 41 54 41 57 48 81 EC A8 00 00 00
		inline const uint64_t OFFSET_H1_GET_NUMBER_OF_TICKS = 0x6F5D90;

		//SCAN: 40 53 56 48 81 EC 38 09 00 00
		inline const uint64_t OFFSET_H1_EXECUTE_COMMAND = 0x7EEA70;
	}

	namespace patch {
		// SCAN: B2 05 41 ?? ?? ?? E8 ?? ?? ?? ?? 84 c0 74 ?? b0 01 48 ?? ?? 28 c3 32 c0 48 ?? ?? 28 C3
		inline const uint64_t OFFSET_ENABLE_DEV_CONSOLE = 0x7811BF;
		inline const auto PATCHBYTES_ENABLE_DEV_CONSOLE = std::vector<uint8_t>{ 0xB0, 0x01 };
	}

	namespace data {
		// TODO-SCALES: Scan for this address when updated
		// SCAN: 88 ?? ?? ?? ?? 01 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 84 C0
		//inline const uint64_t SCAN_CHEATS
		//inline const uint64_t SCAN_OFFSET = 0x6;
		//inline const uint64_t RELATIVE_OFFSET_CHEATS = ...
		inline const uint64_t OFFSET_CHEAT_TABLE = 0x117DA90;
		
		inline const uint64_t OFFSET_TICK_BASE = 0x115D848;
		inline const uint64_t OFFSET_RNG = 0x115DD80;

		// SCAN: E8 ** ** ** ** E8 ** ** ** ** E8 ** ** ** ** E8 ** ** ** ** C7 ** ** ** ** ** ** ** ** ** C6 05 <Relative addr>
		inline const uint64_t OFFSET_CONTROL_FLAGS = 0x115D8C0;
	}

	namespace constants {
		static const float CAMERA_PITCH_MIN = -1.492f;
		static const float CAMERA_PITCH_MAX = 1.492f;
		static const float CAMERA_YAW_MIN = 0.0f;
		static const float CAMERA_YAW_MAX = glm::pi<float>() * 2.0f;
	}

	enum class MAP {
		PILLAR_OF_AUTUMN,
		HALO,
		TRUTH_AND_RECONCILIATION,
		SILENT_CARTOGRAPHER,
		ASSAULT_ON_THE_CONTROL_ROOM,
		_343_GUILTY_SPARK,
		LIBRARY,
		TWO_BETRAYALS,
		KEYES,
		MAW,
		UI_MAIN_MENU,
		UNKNOWN_MAP
	};

	enum class DIFFICULTY : uint8_t {
		EASY,
		NORMAL,
		HEROIC,
		LEGENDARY,
		INVALID_DIFFICULTY
	};

	// KEYS is a layout of the keyboard keys in Halo's memory (one unsigned byte per key)
	// When held, each key increments by 1 for each frame up to the max of 255
	enum class KEYS : uint8_t {
		ESC = 0,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		PrntScrn,
		ScrollLock,
		PauseBreak,
		Tilde,
		N_1,
		N_2,
		N_3,
		N_4,
		N_5,
		N_6,
		N_7,
		N_8,
		N_9,
		N_0,
		Minus,
		Equal,
		Backspace,
		Tab,
		Q,
		W,
		E,
		R,
		T,
		Y,
		U,
		I,
		O,
		P,
		LeftBracket,
		RightBracket,
		Pipe,
		CapsLock,
		A,
		S,
		D,
		F,
		G,
		H,
		J,
		K,
		L,
		Colon,
		Quote,
		Enter,
		LShift,
		Z,
		X,
		C,
		V,
		B,
		N,
		M,
		Comma,
		Period,
		ForwardSlash,
		RShift,
		LCtrl,
		LWin,
		LAlt,
		Space,
		RAlt,
		RWin,
		KeyThatLiterallyNoOneHasEverUsed,
		RCtrl,
		Up,
		Down,
		Left,
		Right,
		Insert,
		Home,
		PageUp,
		Delete,
		End,
		PageDown,
		NumLock,
		NUMPAD_Div,
		NUMPAD_Mul,
		NUMPAD_0,
		NUMPAD_1,
		NUMPAD_2,
		NUMPAD_3,
		NUMPAD_4,
		NUMPAD_5,
		NUMPAD_6,
		NUMPAD_7,
		NUMPAD_8,
		NUMPAD_9,
		NUMPAD_Minus,
		NUMPAD_Plus,
		NUMPAD_Enter,
		NUMPAD_Period,

		KEY_COUNT
	};

	// A subset of KEYS that only contains the most commonly used keys
	const std::vector<KEYS> COMMON_KEYS{
		KEYS::W,
		KEYS::A,
		KEYS::S,
		KEYS::D,
		KEYS::E,
		KEYS::R,
		KEYS::F,
		KEYS::G,
		KEYS::Z,
		KEYS::Tab,
		KEYS::Space,
		KEYS::LCtrl,
		KEYS::LShift,
		KEYS::Q
	};

	enum class COMMON_KEYS : uint8_t {
		W = to_underlying(KEYS::W),
		A = to_underlying(KEYS::A),
		S = to_underlying(KEYS::S),
		D = to_underlying(KEYS::D),

		E = to_underlying(KEYS::E),
		R = to_underlying(KEYS::R),
		F = to_underlying(KEYS::F),
		G = to_underlying(KEYS::G),
		Z = to_underlying(KEYS::Z),
		Tab = to_underlying(KEYS::Tab),
		Space = to_underlying(KEYS::Space),
		LCtrl = to_underlying(KEYS::LCtrl),
		LShift = to_underlying(KEYS::LShift),
		Q = to_underlying(KEYS::Q),
	};

	// Text to display for KEYS codes
	inline extern const std::string KEYS_TO_STRING[] = {
		"ESC",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"PrntScrn",
		"ScrollLock",
		"PauseBreak",
		"Tilde",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"0",
		"Minus",
		"Equal",
		"Backspace",
		"Tab",
		"Q",
		"W",
		"E",
		"R",
		"T",
		"Y",
		"U",
		"I",
		"O",
		"P",
		"[",
		"]",
		"|",
		"CapsLock",
		"A",
		"S",
		"D",
		"F",
		"G",
		"H",
		"J",
		"K",
		"L",
		"Colon",
		"Quote",
		"Enter",
		"LShift",
		"Z",
		"X",
		"C",
		"V",
		"B",
		"N",
		"M",
		",",
		".",
		"/",
		"RShift",
		"LCtrl",
		"LWin",
		"LAlt",
		"Space",
		"RAlt",
		"RWin",
		"Menu",
		"RCtrl",
		"Up",
		"Down",
		"Left",
		"Right",
		"Insert",
		"Home",
		"PageUp",
		"Delete",
		"End",
		"PageDown",
		"NumLock",
		"NUM_/",
		"NUM_*",
		"NUM_0",
		"NUM_1",
		"NUM_2",
		"NUM_3",
		"NUM_4",
		"NUM_5",
		"NUM_6",
		"NUM_7",
		"NUM_8",
		"NUM_9",
		"NUM_-",
		"NUM_+",
		"NUM_ENTER",
		"NUM_."
	};
}

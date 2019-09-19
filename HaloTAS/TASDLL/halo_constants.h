#pragma once

#define HALO_VANILLA
//#define HALO_CUSTOMED

#if defined(HALO_VANILLA) && defined(HALO_CUSTOMED)
#error "Don't define HALO_VANILLA and HALO_CUSTOMED at the same time."
#endif

#if !defined(HALO_VANILLA) && !defined(HALO_CUSTOMED)
#error "Either HALO_VANILLA or HALO_CUSTOMED has to be defined."
#endif

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Patch DirectInput code to allow for editing of mouse x/y values while the game is not in focus
static uint8_t PATCH_DINPUT_MOUSE_BYTES[] = { 0x90,0x90,0x90,0x90,0x90,0x90,0x90 };
static uint8_t PATCH_DINPUT_MOUSE_ORIGINAL[] = { 0x52,0x6A,0x14,0x50,0xFF,0x51,0x24 };

static uint8_t PATCH_FRAME_BEGIN_FUNC_BYTES[] = { 0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90 };
static uint8_t PATCH_FRAME_BEGIN_ORIGINAL_BYTES[sizeof(PATCH_FRAME_BEGIN_FUNC_BYTES)];

static uint8_t PATCH_TICK_BEGIN_ORIGINAL_BYTES[5];
static uint8_t PATCH_TICK_END_ORIGINAL_BYTES[5];

namespace halo::constants {
	static const float CAMERA_PITCH_MIN = -1.492f;
	static const float CAMERA_PITCH_MAX = 1.492f;
	static const float CAMERA_YAW_MIN = 0.0f;
	static const float CAMERA_YAW_MAX = glm::pi<float>() * 2.0f;
}

#if defined(HALO_VANILLA)

namespace halo::function {
	inline extern uintptr_t PRINT_HUD = 0x004AE180; // (int playerIndex, wchar_t* string)
	inline extern uintptr_t GET_TAG_INDEX = 0x00442550; // (int tagTypeIdentifier, char* pathString)

	inline extern uintptr_t PLAY_SOUND_PTR = 0x00549af0;
	typedef uint32_t play_sound_actual_func(uint32_t sndIndex, int16_t*, int, int, int*, int, int);
	inline extern  play_sound_actual_func* PLAY_SOUND = (play_sound_actual_func*)PLAY_SOUND_PTR;
}

namespace halo::addr {

}

inline extern uint32_t* ADDR_RUNTIME_DATA_BEGIN = reinterpret_cast<uint32_t*>(0x40000000);
inline extern uint32_t* ADDR_TAGS_BEGIN = reinterpret_cast<uint32_t*>(0x40440000);

inline extern int32_t* ADDR_FRAMES_SINCE_LEVEL_START_ANIMATION = reinterpret_cast<int32_t*>(0x00746F88);
inline extern int32_t* ADDR_FRAMES_SINCE_LEVEL_START = reinterpret_cast<int32_t*>(0x008603CC);
inline extern int32_t* ADDR_FRAMES_ABSOLUTE = reinterpret_cast<int32_t*>(0x007C3100);
//inline extern int32_t* ADDR_FRAMES_ABSOLUTE_ALTERNATE = reinterpret_cast<int32_t*>(0x007C3104);
inline extern int32_t* ADDR_SIMULATION_TICK = reinterpret_cast<int32_t*>(0x400002F4);
inline extern int32_t* ADDR_SIMULATION_TICK_2 = reinterpret_cast<int32_t*>(0x400002FC);

inline extern uint8_t* ADDR_LOAD_CHECKPOINT = reinterpret_cast<uint8_t*>(0x71973A);
inline extern uint8_t* ADDR_SAVE_CHECKPOINT = reinterpret_cast<uint8_t*>(0x71973F);
inline extern uint8_t* ADDR_RESTART_LEVEL = reinterpret_cast<uint8_t*>(0x719738);
inline extern uint8_t* ADDR_RESTART_LEVEL_FULL = reinterpret_cast<uint8_t*>(0x719739);
inline extern uint8_t* ADDR_CORE_SAVE = reinterpret_cast<uint8_t*>(0x719751);
inline extern uint8_t* ADDR_CORE_LOAD = reinterpret_cast<uint8_t*>(0x719752);

inline extern uint8_t* ADDR_GAME_IS_RUNNING = reinterpret_cast<uint8_t*>(0x400002E9);
inline extern uint8_t* ADDR_GAME_IS_PAUSED = reinterpret_cast<uint8_t*>(0x400002EA);
inline extern float* ADDR_LEFTRIGHTVIEW = reinterpret_cast<float*>(0x402AD4B8);
inline extern float* ADDR_UPDOWNVIEW = reinterpret_cast<float*>(0x402AD4BC);
inline extern char* ADDR_MAP_STRING = reinterpret_cast<char*>(0x40000004);
inline extern uint32_t* ADDR_CHECKPOINT_INDICATOR = reinterpret_cast<uint32_t*>(0x00746F90);
inline extern uint8_t* ADDR_KEYBOARD_INPUT = reinterpret_cast<uint8_t*>(0x006B1620);
inline extern uint8_t* ADDR_LEFTMOUSE = reinterpret_cast<uint8_t*>(0x006B1818);
inline extern uint8_t* ADDR_MIDDLEMOUSE = reinterpret_cast<uint8_t*>(0x006B1819);
inline extern uint8_t* ADDR_RIGHTMOUSE = reinterpret_cast<uint8_t*>(0x006B181A);
inline extern bool* ADDR_SIMULATE = reinterpret_cast<bool*>(0x00721E8C);
inline extern bool* ADDR_ALLOW_INPUT = reinterpret_cast<bool*>(0x006B15F8);

inline extern int32_t* ADDR_DINPUT_MOUSEX = reinterpret_cast<int32_t*>(0x006B180C);
inline extern int32_t* ADDR_DINPUT_MOUSEY = reinterpret_cast<int32_t*>(0x006B1810);
inline extern int32_t* ADDR_DINPUT_MOUSEZ = reinterpret_cast<int32_t*>(0x006B1814); // Scroll

// Patch point for allowing external directinput mouse movement
inline extern uint8_t* ADDR_PATCH_DINPUT_MOUSE = reinterpret_cast<uint8_t*>(0x00490910);
inline extern uint8_t* ADDR_PATCH_FRAME_BEGIN_JUMP_FUNC = reinterpret_cast<uint8_t*>(0x004C7793);
inline extern int32_t* ADDR_FRAME_BEGIN_FUNC_OFFSET = reinterpret_cast<int32_t*>(0x004C7798);
inline extern bool* ADDR_RUN_FRAME_BEGIN_CODE = reinterpret_cast<bool*>(0x0071D1A4);

inline extern int32_t* ADDR_FRAME_BEGIN_REPLACED_FUNC = reinterpret_cast<int32_t*>(0x004C6E80);

inline extern int32_t* ADDR_TICK_BEGIN_FUNC_OFFSET = reinterpret_cast<int32_t*>(0x0045B79B);
inline extern int32_t* ADDR_TICK_REPLACED_FUNC = reinterpret_cast<int32_t*>(0x0045B590);
			  
inline extern int32_t* ADDR_TICK_END_FUNC_OFFSET = reinterpret_cast<int32_t*>(0x0045B837);
inline extern int32_t* ADDR_TICK_END_REPLACED_FUNC = reinterpret_cast<int32_t*>(0x00456730);

inline extern glm::vec3* ADDR_CAMERA_POSITION = reinterpret_cast<glm::vec3*>(0x006AC6D0);
inline extern float* ADDR_CAMERA_LOOK_VECTOR = reinterpret_cast<float*>(0x006AC72C);
inline extern float** ADDR_PTR_TO_CAMERA_HORIZONTAL_FIELD_OF_VIEW_IN_RADIANS = reinterpret_cast<float**>(0x00445920);
inline extern float* ADDR_PLAYER_YAW_ROTATION_RADIANS   = reinterpret_cast<float*>(0x402AD4B8);
inline extern float* ADDR_PLAYER_PITCH_ROTATION_RADIANS = reinterpret_cast<float*>(0x402AD4BC);
inline extern uint8_t* ADDR_DEBUG_CAMERA_ENABLE = reinterpret_cast<uint8_t*>(0x006AC568);
//inline extern uint8_t* debugB = reinterpret_cast<uint8_t*>(0x006AC569);

inline extern float* ADDR_GAME_SPEED = reinterpret_cast<float*>(0x40000300);
inline extern int32_t* ADDR_RNG = reinterpret_cast<int32_t*>(0x00719CD0);

#elif defined(HALO_CUSTOMED)

inline extern uint32_t* ADDR_TAGS_ARRAY = (uint32_t*)0x40000000;
inline extern uint32_t TAG_ARRAY_LENGTH_BYTES = 0x440000;

inline extern int32_t* ADDR_FRAMES_SINCE_LEVEL_START_ANIMATION = (int32_t*)0x00746F88;
inline extern int32_t* ADDR_INPUT_TICK = (int32_t*)0x006F1D8C;
inline extern float* ADDR_LEFTRIGHTVIEW = (float*)0x402AD4B8;
inline extern float* ADDR_UPDOWNVIEW = (float*)0x402AD4BC;
inline extern char* ADDR_MAP_STRING = (char*)0x006A8174;
inline extern uint32_t* ADDR_CHECKPOINT_INDICATOR = (uint32_t*)0x00746F90;
inline extern uint8_t* ADDR_KEYBOARD_INPUT = (uint8_t*)0x006B1620;
inline extern uint8_t* ADDR_LEFTMOUSE = (uint8_t*)0x006B1818;
inline extern uint8_t* ADDR_MIDDLEMOUSE = (uint8_t*)0x006B1819;
inline extern uint8_t* ADDR_RIGHTMOUSE = (uint8_t*)0x006B181A;
inline extern bool* ADDR_SIMULATE = (bool*)0x006BD15C;
inline extern bool* ADDR_ALLOW_INPUT = (bool*)0x0064C528;

inline extern int32_t* ADDR_DINPUT_MOUSEX = (int32_t*)0x006B180C;
inline extern int32_t* ADDR_DINPUT_MOUSEY = (int32_t*)0x006B1810;
inline extern int32_t* ADDR_DINPUT_MOUSEZ = (int32_t*)0x006B1814; // Scroll

// Patch point for allowing external directinput mouse movement
inline extern uint8_t* ADDR_PATCH_DINPUT_MOUSE = (uint8_t*)0x00490910;
inline extern float* ADDR_CAMERA_POSITION = (float*)0x00647600;
inline extern float* ADDR_CAMERA_LOOK_VECTOR = (float*)0x0064765C;
inline extern float** ADDR_PTR_TO_CAMERA_HORIZONTAL_FIELD_OF_VIEW_IN_RADIANS = (float**)0x00446280;

inline extern float* ADDR_GAME_SPEED = (float*)0x40000300;

#endif

// KEYS is a layout of the keyboard keys in Halo's memory (one unsigned byte per key)
// When held, each key increments by 1 for each frame up to the max of 255
enum KEYS {
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
	NUM_1,
	NUM_2,
	NUM_3,
	NUM_4,
	NUM_5,
	NUM_6,
	NUM_7,
	NUM_8,
	NUM_9,
	NUM_0,
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
	RightCtrl,
	Up,
	Down,
	Left,
	Right,

	KEY_COUNT
};

// Text to display for KEYS codes
inline extern std::string KEY_PRINT_CODES[] = {
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
	"NUM_.",
};

struct ObjectPoolObject
{
	uint32_t ObjectAddress;
	uint32_t unk1;
	uint32_t unk2;
};

inline extern int8_t MAGIC_DATAPOOLHEADER[] = {/* 0x00, 0x08, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00,*/ 0x40, 0x74, 0x40, 0x64 };
struct DataPool {
	char Name[32];
	uint32_t unk1; // 000C0800
	uint32_t unk2; // 00000001
	uint32_t DataHeader; // 0x64407440 @t@d
	int16_t UnkCounter1;
	int16_t ObjectCount;
	int16_t UnkCounter3;
	int16_t UnkCounter4; // Total objects ever?
	uint32_t DataBeginAddress;
	uint32_t unk3;
	uint32_t unk4;

	ObjectPoolObject ObjectPointers[8192];
};

struct Tag {
	uint32_t id;
	glm::vec3 displayColor;
	std::string displayName;
};

const std::unordered_map<uint32_t, Tag> KNOWN_TAGS = {
{ 580, Tag{ 580, glm::vec3(0,1,0), "static prop" } },
{ 616, Tag{ 616, glm::vec3(0,1,0), "Terminal" }},
{ 628, Tag{ 628, glm::vec3(0,1,0), "bulkhead?"   }}, 
{ 632, Tag{ 632, glm::vec3(0,1,0), "tree"   }}, 
{ 680, Tag{ 680, glm::vec3(0,1,0), "animated light"   }}, 
{ 720, Tag{ 720, glm::vec3(0,1,0), "debris" } },
{ 732, Tag{ 732, glm::vec3(0,1,0), "door"   }},
{ 736, Tag{ 736, glm::vec3(0,1,0), "tree"   }}, 
{ 764, Tag{ 764, glm::vec3(0,1,0), "projectile"   }}, 
{ 800, Tag{ 800, glm::vec3(0,1,0), "Ammo/Health/Consumable"   }}, 
{ 836, Tag{ 836, glm::vec3(0,1,0), "bulkhead?"   }}, 
{ 892, Tag{ 892, glm::vec3(0,1,0), "trigger?"   }}, 
{ 940, Tag{ 940, glm::vec3(0,1,0), "Door" } },
{ 972, Tag{ 972, glm::vec3(0,1,0), "Needler" } },
{ 1088,Tag{ 1088, glm::vec3(0,1,0), "Shotgun"   }},
{ 1204,Tag{ 1204, glm::vec3(0,1,0), "MA5B"  }}, 
{ 1320,Tag{ 1320, glm::vec3(0,1,0), "Plasma Pistol"  }}, 
{ 1356,Tag{ 1356, glm::vec3(0,1,0), "POA Terminal"  }}, 
{ 1436,Tag{ 1436, glm::vec3(0,1,0), "Plasma Rifle/Rockets"  }}, 
{ 1612,Tag{ 1612, glm::vec3(0,1,0), "POA Bridge Chair"  }},
{ 1616,Tag{ 1616, glm::vec3(0,1,0), "343 GS"  }},
{ 1668,Tag{ 1668, glm::vec3(0,1,0), "Pistol"  }},
{ 1728,Tag{ 1728, glm::vec3(0,1,0), "???"  }}, 
{ 1960,Tag{ 1960, glm::vec3(0,1,0), "Banshee" } },
{ 2076,Tag{ 2076, glm::vec3(0,1,0), "Ghost" } },
{ 2424,Tag{ 2424, glm::vec3(0,1,0), "Wraith" } },
{ 2888,Tag{ 2888, glm::vec3(0,1,0), "Pelican" } },
{ 2892,Tag{ 2892, glm::vec3(0,1,0), "Popcorn"  }},
{ 3356,Tag{ 3356, glm::vec3(0,1,0), "Grunt" } },
{ 3584,Tag{ 3584, glm::vec3(0,1,0), "Warthog?"  }}, 
{ 3588,Tag{ 3588, glm::vec3(0,1,0), "You!"  }}, 
{ 3704,Tag{ 3704, glm::vec3(0,1,0), "Marine" } },
{ 3936,Tag{ 3936, glm::vec3(0,1,0), "Infection Form"  }},
{ 4400,Tag{ 4400, glm::vec3(0,1,0), "Elite"  }}, 
{ 4516,Tag{ 4516, glm::vec3(0,1,0), "Jackal"  }},
{ 4864,Tag{ 4864, glm::vec3(1,0,1), "Hunter"  }},
{ 5328,Tag{ 5328, glm::vec3(0,1,0), "Cpt. Keyes" } },
{ 5792,Tag{ 5792, glm::vec3(0,1,0), "Flood (human)" } },
{ 6024,Tag{ 6024, glm::vec3(0,1,0), "Flood (elite)" }},
};

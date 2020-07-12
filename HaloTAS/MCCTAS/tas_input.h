#pragma once
#include "pch.h"
#include <optional>

const uint32_t MOUSE_BUTTON_LEFT = 0x1;
const uint32_t MOUSE_BUTTON_MIDDLE = 0x2;
const uint32_t MOUSE_BUTTON_RIGHT = 0x4;
const uint32_t MOUSE_BUTTON_4 = 0x8;
const uint32_t MOUSE_BUTTON_5 = 0x16;

// Gamepad Buttons
const uint16_t XINPUT_GAMEPAD_DPAD_UP = 0x0001;
const uint16_t XINPUT_GAMEPAD_DPAD_DOWN = 0x0002;
const uint16_t XINPUT_GAMEPAD_DPAD_LEFT = 0x0004;
const uint16_t XINPUT_GAMEPAD_DPAD_RIGHT = 0x0008;
const uint16_t XINPUT_GAMEPAD_START = 0x0010;
const uint16_t XINPUT_GAMEPAD_BACK = 0x0020;
const uint16_t XINPUT_GAMEPAD_LEFT_THUMB = 0x0040;
const uint16_t XINPUT_GAMEPAD_RIGHT_THUMB = 0x0080;
const uint16_t XINPUT_GAMEPAD_LEFT_SHOULDER = 0x0100;
const uint16_t XINPUT_GAMEPAD_RIGHT_SHOULDER = 0x0200;
const uint16_t XINPUT_GAMEPAD_A = 0x1000;
const uint16_t XINPUT_GAMEPAD_B = 0x2000;
const uint16_t XINPUT_GAMEPAD_X = 0x4000;
const uint16_t XINPUT_GAMEPAD_Y = 0x8000;

struct MCCInput {
	// +000
	uint8_t InputTypeFlag; // 0 = Controller, 1 = Mouse+Keyboard
	uint8_t Padding1[3];

	////////////////////////// Keyboard + Mouse
	// +004
	uint8_t VKeyTable[256];
	// +260
	float MouseDeltaX;
	float MouseDeltaY;
	float MouseDeltaScroll;
	float UnknownFloat; // 1.0 when using M+K, 0 otherwise?
	uint32_t MouseButtonFlags;
	////////////////////////// Gamepad // Vaguely similar to XINPUT_GAMEPAD structure
	// +280
	uint16_t GamepadButtonFlags;
	uint16_t Padding2;
	uint8_t GamepadLeftTrigger;
	uint8_t GamepadRightTrigger;
	int16_t GamepadThumbLeftX;
	int16_t GamepadThumbLeftY;
	int16_t GamepadThumbRightX;
	int16_t GamepadThumbRightY;
	// Possibly more after this...
};

struct tick_inputs {
	int32_t mAbsoluteTick;
	int32_t mRNGStart;
	std::vector<MCCInput> mInputs;
};

struct input_return {
	MCCInput input;
	bool isLastFrame;
};
class tas_input
{
	std::vector<tick_inputs> mLevelInput;
	int32_t mCurrentTick = 0;
	tick_inputs mWorkingInputs;
public:

	void start_tick(int RNG);
	void push_input(const MCCInput& input);
	void end_tick();
	
	void reset();

	
	std::optional<input_return> get_input(int32_t tick, int32_t frame, int32_t RNG);
};


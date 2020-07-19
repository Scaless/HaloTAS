#include "pch.h"
#include "tas_input.h"

void tas_input::start_tick(int RNG)
{
	mWorkingInputs.mRNGStart = RNG;
	mWorkingInputs.mAbsoluteTick = mCurrentTick;
}

void tas_input::push_input(const MCCInput& input)
{
	mWorkingInputs.mInputs.push_back(input);
}

void tas_input::end_tick()
{
	mLevelInput.push_back(mWorkingInputs);
	mWorkingInputs = tick_inputs();
}

void tas_input::reset()
{
	mLevelInput.clear();
	mCurrentTick = 0;
	mWorkingInputs = tick_inputs();
}

std::optional<input_return> tas_input::get_input(int32_t tick, int32_t frame, int32_t RNG)
{
	if (tick < mLevelInput.size() - 1) {
		auto tickInputs = mLevelInput[tick];

		input_return ir;
		ir.rng = tickInputs.mRNGStart;
		if (RNG != tickInputs.mRNGStart) {
			ir.rngError = true;
			tas_logger::warning("Tick({}): RNG does not match: Expected({}) / Actual({})", tick, tickInputs.mRNGStart, RNG);
		}

		if (frame < tickInputs.mInputs.size()) {
			ir.input = tickInputs.mInputs[frame];

			if (frame == tickInputs.mInputs.size() - 1) {
				ir.isLastFrame = true;
			}
			else {
				ir.isLastFrame = false;
			}
			return ir;
		}
	}

	return std::nullopt;
}

//void tick_inputs::flatten()
//{
//	MCCInput Flattened{};
//	for (auto& i : mInputs) {
//		Flattened.InputTypeFlag = i.InputTypeFlag;
//		Flattened.MouseDeltaX += i.MouseDeltaX;
//		Flattened.MouseDeltaY += i.MouseDeltaY;
//
//		for (int q = 0; q < sizeof(i.VKeyTable); q++) {
//			Flattened.VKeyTable[q] = std::max<uint8_t>(Flattened.VKeyTable[q], i.VKeyTable[q]);
//		}
//
//		Flattened.MouseButtonFlags &= i.MouseButtonFlags;
//	}
//	mInputs.clear();
//	mInputs.push_back(Flattened);
//}


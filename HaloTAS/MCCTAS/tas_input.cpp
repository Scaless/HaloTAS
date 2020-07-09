#include "tas_input.h"

void tas_input::add(const tick_inputs& inputs)
{
	mLevelInput.push_back(inputs);
}

void tas_input::clear()
{
	mLevelInput.clear();
}

int tas_input::tick_count()
{
	return mLevelInput.size();
}

int tas_input::input_count()
{
	int count = 0;
	for (auto& i : mLevelInput) {
		count += i.count();
	}
	return count;
}

const tick_inputs& tas_input::get_inputs_at_tick(int32_t tick)
{
	return mLevelInput[tick];
}

void tick_inputs::add(const MCCInput& input)
{
	mInputs.push_back(input);
}

void tick_inputs::clear()
{
	mInputs.clear();
}

const MCCInput& tick_inputs::get_input_at_frame(int frame)
{
	if (frame < mInputs.size()) {
		return mInputs[frame];
	}
}

int tick_inputs::count()
{
	return mInputs.size();
}

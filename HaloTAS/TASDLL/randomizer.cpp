#include "randomizer.h"

void randomizer::set_seed(int32_t seed)
{
	eng.seed(seed);
}

void randomizer::start_randomizer()
{
	active = true;
}

void randomizer::stop_randomizer()
{
	active = false;
}

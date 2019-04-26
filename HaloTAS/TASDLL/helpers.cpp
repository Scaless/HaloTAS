#include "helpers.h"

int32_t next_rng(int32_t current_rng) {
	return 1664525 * current_rng + 1013904223;
}

double rng_to_double(int32_t rng)
{
	return (int32_t((uint32_t)rng >> 16u)) * 0.000015259022;
}


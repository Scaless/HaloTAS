#include "helpers.h"

int32_t next_rng(int32_t current_rng) {
	return 1664525 * current_rng + 1013904223;
}

double rng_to_double(int32_t rng)
{
	return (int32_t((uint32_t)rng >> 16u)) * 0.000015259022;
}

int32_t calc_rng_count(int32_t start_rng, int32_t end_rng, uint32_t max_change = 5000) {

	int32_t count = 0;
	int32_t current_rng = start_rng;
	for (uint32_t i = 0; i < max_change; i++) {
		if (current_rng == end_rng) {
			return count;
		}
		current_rng = next_rng(current_rng);
		count++;
	}

	return -1;
}

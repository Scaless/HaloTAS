#pragma once

#include <inttypes.h>

// Gets the next RNG value in the sequence
int32_t next_rng(int32_t current_rng);

// Converts RNG value to a double in the same way that Halo does
double rng_to_double(int32_t rng);

// Calculates the count of RNG iterations between two RNG values, up to a maximum
int32_t calc_rng_count(int32_t start_rng, int32_t end_rng, uint32_t max_change);
#pragma once
#include <d3d11.h>
#include <vector>
#include "tas_input.h"

namespace tas::overlay {
	void initialize(IDXGISwapChain* SwapChain);
	void render(const tas_input& Input);
	void shutdown();

	void add_speed_value(float speed);
}

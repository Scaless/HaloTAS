#pragma once
#include <d3d11.h>

namespace tas::overlay {
	void initialize(IDXGISwapChain* SwapChain);
	void render();
	void shutdown();
}

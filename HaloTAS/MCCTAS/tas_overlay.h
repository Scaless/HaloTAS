#pragma once
#include "pch.h"
#include "tas_input.h"
#include "speedometer.h"

namespace tas::overlay {
	void initialize(IDXGISwapChain* SwapChain);
	void render(const tas_input& Input);
	void shutdown();

	void set_current_speedometer(std::shared_ptr<speedometer> spm);
}

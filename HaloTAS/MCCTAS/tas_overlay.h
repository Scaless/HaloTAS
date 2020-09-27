#pragma once
#include "pch.h"
#include "tas_input.h"
#include "speedometer.h"

namespace tas::overlay {
	void initialize(IDXGISwapChain* SwapChain);
	void render(const tas_input& Input);
	void shutdown();

	bool should_render(int index_count);
	void set_wireframe();
	void set_wireframe(ID3D11DeviceContext* Ctx, int index_count);
	void set_normal(ID3D11DeviceContext* Ctx);
	void set_current_speedometer(std::shared_ptr<speedometer> spm);
}

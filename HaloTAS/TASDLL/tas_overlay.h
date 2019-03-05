#pragma once

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "halo_engine.h"
#include "render_cube.h"
#include "render_text.h"

struct tas_overlay_input
{
	bool overlayClose = false;
	bool record = false;
	bool playback = false;
	bool loadPlayback = false;
	bool showPrimitives = false;
	bool forceSimulate = true;

	float cullDistance = 15, UIScale = 1.5f;
};

class tas_overlay
{
private:
	GLFWwindow* window;
	ImGuiContext* imguiCtx;
	std::unique_ptr<render_text> textRenderer;
	std::unique_ptr<render_cube> cubeRenderer;

	tas_overlay_input currentInput;

public:
	tas_overlay();
	~tas_overlay();

	GLFWwindow* glfw_window();
	void make_context_current();
	void update_position(HWND haloWindow);

	const tas_overlay_input& render_and_get_input(halo_engine& engine);
};


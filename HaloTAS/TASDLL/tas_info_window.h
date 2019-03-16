#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "halo_engine.h"
#include "tas_overlay.h"

struct tas_info_input
{
	bool overlayClose = false;
	bool record = false;
	bool playback = false;
	bool loadPlayback = false;
	bool forceSimulate = true;

	tas_overlay_render_options overlayOptions;
};

class tas_info_window
{
private:
	GLFWwindow* window;
	ImGuiContext* imguiCtx;
	ImFontAtlas* atlas;
	bool close;

	tas_info_input currentInput;

	void render_overlay(halo_engine& engine);
	void render_tas(halo_engine& engine);
	void render_menubar(halo_engine& engine);

public:
	tas_info_window();
	~tas_info_window();

	void render(halo_engine& engine);
	tas_info_input& getInput();
	bool shouldClose();
};


#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
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
	bool updateViewPos = true;
	bool disable_cutscene_fps_cap = true;
	bool disable_look_centering = true;

	tas_overlay_render_options overlayOptions;
};

class tas_info_window
{
private:
	GLFWwindow* window;
	ImGuiContext* imguiCtx;
	ImFontAtlas* atlas;
	bool close {false};

	tas_info_input currentInput;

	void render_overlay();
	void render_tas();
	void render_d3d();
	void render_inputs();
	void render_menubar();
	void render_rng();
	void render_other();
	void render_randomizer();
	void render_tags();

	void render_imgui();

public:
	tas_info_window();
	~tas_info_window();

	void render();
	tas_info_input& getInput();
	bool shouldClose();
};


#pragma once

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "halo_engine.h"
#include "render_cube.h"
#include "render_text.h"

struct tas_overlay_render_options
{
	bool enabled = true;
	float cullDistance = 15.0f;
	float uiScale = 1.5f;
	bool showPrimitives = true;
	HWND haloWindow = NULL;
};

class tas_overlay
{
private:
	GLFWwindow* window;
	std::unique_ptr<render_text> textRenderer;
	std::unique_ptr<render_cube> cubeRenderer;
	bool focused = false;

	void glfwMouseButtonFunc(GLFWwindow* w, int button, int action, int mods);
	void update_position(HWND haloWindow);

public:
	tas_overlay();
	~tas_overlay();

	void render(halo_engine& engine, const tas_overlay_render_options& options);
};


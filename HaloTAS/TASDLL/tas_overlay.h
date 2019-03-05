#pragma once

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <memory>

class tas_overlay
{
private:
	GLFWwindow* window;

public:
	tas_overlay();
	~tas_overlay();

	GLFWwindow* glfw_window();
	void make_context_current();
};


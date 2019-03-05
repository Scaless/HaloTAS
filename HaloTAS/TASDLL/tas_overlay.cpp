#include "tas_overlay.h"

tas_overlay::tas_overlay()
{
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

	window = glfwCreateWindow(1280, 720, "Halo TAS Tools", NULL, NULL);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
}

tas_overlay::~tas_overlay()
{
	glfwDestroyWindow(window);
}

GLFWwindow* tas_overlay::glfw_window()
{
	return window;
}

void tas_overlay::make_context_current()
{
	glfwMakeContextCurrent(window);
}



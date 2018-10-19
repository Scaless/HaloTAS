#pragma once

#include <GL/gl3w.h>

#include <dwmapi.h>
#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

static void glfw_error_callback(int error, const char* description);

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);
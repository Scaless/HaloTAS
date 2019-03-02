#pragma once

#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include "render_opengl.h"

class render_cube
{
private:
	const static GLfloat unit_cube_vertex_buffer[];
	GLuint programId;
	GLuint vertexArrayId, vertexBuffer;

public:
	render_cube();
	~render_cube();

	void initialize();
	void draw_cube(glm::mat4 proj, glm::mat4 view, glm::vec3 pos, float scale, glm::vec3 color);
};


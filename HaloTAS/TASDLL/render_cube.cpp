#include "render_cube.h"
#include <glm/gtc/matrix_transform.hpp>

const GLfloat render_cube::unit_cube_vertex_buffer[] = {
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
};

render_cube::render_cube()
{
	initialize();
}

render_cube::~render_cube()
{
}

void render_cube::initialize()
{
	glGenVertexArrays(1, &vertexArrayId);
	glBindVertexArray(vertexArrayId);
	
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(unit_cube_vertex_buffer), unit_cube_vertex_buffer, GL_STATIC_DRAW);

	programId = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");
}

void render_cube::draw_cube(glm::mat4 projection, glm::mat4 view, glm::vec3 pos, float scale, glm::vec3 color)
{
	glm::mat4 model, mvp;
	model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	model = glm::scale(model, glm::vec3(scale));
	mvp = projection * view * model;

	glUseProgram(programId);
	glEnableVertexAttribArray(0);
	glBindVertexArray(vertexArrayId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
	glUniformMatrix4fv(glGetUniformLocation(programId, "MVP"), 1, GL_FALSE, &mvp[0][0]);
	glUniform3f(glGetUniformLocation(programId, "fixedColor"), color.x, color.y, color.z);

	// Draw the triangle !
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

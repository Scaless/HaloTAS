#pragma once

#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H 
#include "render_opengl.h"
#include <unordered_map>

struct ft_character {
	GLuint     TextureID;  // ID handle of the glyph texture
	glm::ivec2 Size;       // Size of glyph
	glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
	GLuint     Advance;    // Offset to advance to next glyph
};

class render_text
{
private:
	std::unordered_map<GLchar, ft_character> Characters;
	GLuint VAO, VBO;
	GLuint TextProgram;

public:
	render_text();
	~render_text();

	void initialize();
	void draw_text(std::string text, GLfloat scale, glm::vec3 color, glm::mat4 projection, glm::mat4 view, glm::vec3 pos, glm::vec3 playerPos);
};
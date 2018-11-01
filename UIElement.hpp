#pragma once

#include <string>
#include <functional>
#include <array>

#include <glm/glm.hpp>
#include "GL.hpp"

#define GL_SILENCE_DEPRECATION()
struct UIElement {
public:

	std::function<void(glm::vec2)> onHover;
	std::function<void(glm::vec2, int)> onMouseDown;
	std::function<void(glm::vec2, int)> onMouseUp;
	std::function<void(glm::vec2, int)> onMouse;

	virtual void draw(glm::uvec2 const& drawable_size){};
	virtual void update(float elapsed){};

	GLuint vbo;
	GLuint vao;
	bool smooth = false;
};

struct UILabel : UIElement {
public:
	glm::vec2 center;
	glm::uvec2 dim;
	std::string text;

	UILabel(glm::vec2 const& center, glm::uvec2 const& dim, std::string const& text) : center(center), dim(dim), text(text) {}
};

struct UIBox : UIElement {
public:
	glm::vec2 pos;
	glm::vec2 size;
	glm::u8vec4 color;
	bool active = false;

	UIBox(glm::vec2 p, glm::vec2 s, glm::u8vec4 c);

	void draw(glm::uvec2 const& drawable_size);
	void on_mouse_move(glm::vec2 mouse);

	struct button_vertex {
		glm::vec3 position;
		glm::u8vec4 color;
		glm::vec2 texcoord;
	};
	std::array<button_vertex, 6> verts_buf;
};
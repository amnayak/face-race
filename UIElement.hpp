#pragma once

#include <string>
#include <functional>

#include <glm/glm.hpp>

struct UIElement {
public:
	std::function<void(glm::vec2)> onHover;
	std::function<void(glm::vec2, int)> onMouseDown;
	std::function<void(glm::vec2, int)> onMouseUp;
	std::function<void(glm::vec2, int)> onMouse;

	virtual void draw(glm::uvec2 const& drawable_size) = 0;
	virtual void update(float elapsed) = 0;
};

struct UILabel : UIElement {
public:
	UILabel(std::string const& text, glm::vec2 const& center, glm::uvec2 const& dim) : center(center), dim(dim), text(text) {}

	std::string text;
	glm::vec2 center;
	glm::uvec2 dim;
};
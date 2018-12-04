#pragma once

#include <string>
#include <functional>
#include <vector>
#include <array>

#include <glm/glm.hpp>
#include <SDL.h>
#include "GL.hpp"

// Shader Program
struct UIProgram {
	GLuint program;

	GLuint u_texture;
	GLuint u_projTrans;
	GLuint u_color;

	GLuint in_position;
	GLuint in_texcoord0;
};

enum HozAnchor { Left, Center, Right };
enum VrtAnchor { Top, Middle, Bottom};

struct UIElement {
public:
	std::function<void(glm::vec2 const&, bool)> onHover;
	std::function<void(glm::vec2 const&, bool, int)> onMouseDown;
	std::function<void(glm::vec2 const&, bool, int)> onMouseUp;
	std::function<void(glm::vec2 const&, glm::vec2 const&)> onResize;

	glm::vec2 pos; // center
	glm::vec2 size; // dims

	std::string name = "unnamed";
	GLuint tex;

	bool enabled = true; // if false, do not draw or update

	UIElement(glm::vec2 pos, glm::vec2 size) : pos(pos), size(size) { }

	virtual void handle_event(SDL_Event const &evt, glm::uvec2 const &window_size);

	virtual void draw(glm::uvec2 const& window_size, glm::mat4 const& view) = 0;
	virtual void update(float elapsed) {
		if(onUpdate)
			onUpdate(elapsed);
	}

	static UIElement *create_slider(
		glm::vec2 center,
		float width,
		float bar_thickness,
		float btn_thickness,
		std::function<void(float)> value_changed,
		std::function<float(void)> retrieve_value,
		float initial,
		HozAnchor hoz = Left,
    	VrtAnchor vrt = Top);

	static UIElement *ui_element_focused;

protected:
	std::function<void(float)> onUpdate;
	glm::uvec2 prev_window_size = glm::uvec2(0,0);
};

struct UIGroupElement : public UIElement {
public:
	const std::vector<UIElement *> children;

	UIGroupElement(const std::vector<UIElement *> children, glm::vec2 pos, glm::vec2 size) : UIElement(pos, size), children(children) {
		onResize = [this](glm::vec2 const &s1, glm::vec2 const &s2){
			for(UIElement *cur : this->children)
				if(cur->onResize)
					cur->onResize(s1, s2);
		};
	}

	virtual void handle_event(SDL_Event const &evt, glm::uvec2 const &window_size);
	void draw(glm::uvec2 const& window_size, glm::mat4 const& view);
	void update(float elapsed);
};

struct UILabel : public UIElement {
public:
	std::string text;

	UILabel(glm::vec2 const& pos, glm::uvec2 const& size, std::string const& text) : UIElement(pos, size), text(text) {}

	void draw(glm::uvec2 const& window_size, glm::mat4 const& view);
};

struct UIBox : public UIElement {
public:
	glm::vec4 color = glm::vec4(1,1,1,0.7f);
	bool active = false;

	UIBox(glm::vec2 pos, glm::vec2 size, glm::vec4 color);

	void draw(glm::uvec2 const& window_size, glm::mat4 const& view);

private:
	GLuint vbo;
	GLuint vao;
	struct button_vertex {
		glm::vec2 position;
		glm::vec2 texcoord;
	};
	std::array<button_vertex, 6> verts_buf;
};
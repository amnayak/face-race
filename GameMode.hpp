#pragma once

#include "Mode.hpp"
#include "ShapeKeyMesh.hpp"
#include "UIElement.hpp"

#include "MeshBuffer.hpp"
#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

// The 'GameMode' mode is the main gameplay mode:

struct GameMode : public Mode {
	GameMode(glm::uvec2 const& window_size);
	virtual ~GameMode();

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;

	//update is called at the start of a new frame, after events are handled:
	virtual void update(float elapsed) override;

	//draw is called after update:
	virtual void draw(glm::uvec2 const &drawable_size) override;

	float camera_spin = 0.0f;
	float spot_spin = 0.0f;

	ShapeKeyMesh *face;
	std::vector<float> weights;

	glm::uvec2 window_size;
	Scene::Object *cube = nullptr;
	std::vector<UIElement *> ui_elements;
};

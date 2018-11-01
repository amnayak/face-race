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

#define BASIS			0
#define EYE_DOWN		1
#define EYE_UP			12
#define EYE_LOOK_R		5
#define EYE_LOOK_L		10

#define BROW_L_UP		13
#define BROW_R_UP		2
#define BROW_L_DWN		8
#define BROW_R_DWN		0xF

// eyelids are getting weird contributions, check mesh later
#define EYE_L_CLOSE		3
#define EYE_R_CLOSE		9

#define MOUTH_L_UP		4
#define MOUTH_R_UP		11
#define MOUTH_L_DWN		0x0F
#define MOUTH_R_DWN		7
#define MOUTH_OPEN		15
// The 'GameMode' mode is the main gameplay mode:

struct GameMode : public Mode {
	GameMode();
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
	UIBox *eye_handle;
	UIBox *brow_l_handle;
	UIBox *brow_r_handle;
	UIBox *mouth_handle;
};

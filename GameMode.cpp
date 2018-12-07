#include "GameMode.hpp"

#include "MenuMode.hpp"
#include "Load.hpp"
#include "cube_program.hpp"
#include "cube_diffuse_program.hpp"
#include "cube_reflect_program.hpp"
#include "make_vao_for_program.hpp"
#include "load_save_png.hpp"
#include "rgbe.hpp"

#include "MeshBuffer.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp" //helper for dumpping OpenGL error messages
#include "check_fb.hpp" //helper for checking currently bound OpenGL framebuffer
#include "read_chunk.hpp" //helper for reading a vector of structures from a file
#include "data_path.hpp" //helper to get paths relative to executable
#include "compile_program.hpp" //helper to compile opengl shader programs
#include "draw_text.hpp" //helper to... um.. draw text
#include "load_save_png.hpp"
#include "texture_program.hpp"
#include "depth_program.hpp"
#include "Font.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>
#include <sstream>
#include <chrono>
#include <array>
typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::duration<float> fsec;
auto t0 = Time::now();
auto t1 = Time::now();

//TODO: hacky garbage right here
uint32_t GAME_OVER = 0;
uint32_t WAIT_FOR_INPUT = 1;
uint32_t CHECK = 3;

const uint32_t MENU = 5;
const uint32_t HAPPY = 6;
const uint32_t SAD = 7;
const uint32_t CONFUSED = 8;
const uint32_t SURPRISE = 9;
const uint32_t NEUTRAL = 10;

bool did_happy = false;
bool happy = false;
bool sad = false;
bool gg = false;

uint32_t posedge_clock = 1;
uint32_t state = WAIT_FOR_INPUT;
uint32_t game_state = MENU;

uint32_t states[] = {HAPPY, SAD, CONFUSED, SURPRISE};

std::string happy_strs[] = {
	"This is the best first date I've had in a while, thanks for being you",
	"You're a really good listener, I appreciate that in a guy.",
	"I really enjoy Christmas time... The lights, the snow, the music. It just makes me feel happy",
	"I love your hair, where do you get it cut?"
};

std::string sad_strs[] = {
	"I love dogs. I just really miss my childhood dog. He was always so happy, no matter what",
	"I haven't slept well in weeks. My studies are really starting to get to me.",
	"Do you have seasonal depression? Sometimes I think that I might. The lack of sunlight really gets to me.",
};
std::string conf_strs[] = {
	"How would you implement image based lighting? What? Do you not usually talk about computer graphics on the first date?",
	"Would you rather eat ice cream with a fork or a knife?",
	"I like Fortnite. Wait-what do you mean you don't play Fortnite?"
};
std::string supr_strs[] = {
	"I just choked on my asparagus, oh my goodness.",
	"Wait a minute, you went to the graphics conference last year? No way, so did I!",
	"I never tip my servers 15 percent. In my opinion, the restaurants should pay them more and not make it my responsibility.",
	"You know what, I'll take the check. I like to switch things up sometimes!"
};



static MeshBuffer *suzanne_mesh;
// Key: name of mesh
// Value: index into meshes / meshes_for_***_program
static std::map<std::string, unsigned short> mesh2idx;

//load an rgbe cubemap texture:
GLuint load_cube(std::string const &filename) {
	//assume cube is stacked faces +x,-x,+y,-y,+z,-z:
	glm::uvec2 size;
	std::vector< glm::u8vec4 > data;
	load_png(filename, &size, &data, LowerLeftOrigin);
	if (size.y != size.x * 6) {
		throw std::runtime_error("Expecting stacked faces in cubemap.");
	}

	//convert from rgb+exponent to floating point:
	std::vector< glm::vec3 > float_data;
	float_data.reserve(data.size());
	for (auto const &px : data) {
		float_data.emplace_back(rgbe_to_float(px));
	}

	//upload to cubemap:
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	//the RGB9_E5 format is close to the source format and a lot more efficient to store than full floating point.
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 0*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 1*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 2*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 3*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 4*size.x*size.x);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB9_E5, size.x, size.x, 0, GL_RGB, GL_FLOAT, float_data.data() + 5*size.x*size.x);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//this will probably be ignored because of GL_TEXTURE_CUBE_MAP_SEAMLESS:
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	//NOTE: turning this on to enable nice filtering at cube map boundaries:
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	GL_ERRORS();

	return tex;
}

Load< GLuint > sky_cube(LoadTagDefault, [](){
	return new GLuint(load_cube(data_path("cape_hill_512.png")));
});

Load< GLuint > diffuse_cube(LoadTagDefault, [](){
	return new GLuint(load_cube(data_path("cape_hill_diffuse.png")));
});

uint32_t cube_mesh_count = 0;
Load< GLuint > cube_mesh_for_cube_program(LoadTagDefault, [](){
	//mesh for showing cube map texture:
	glm::vec3 v0(-1.0f,-1.0f,-1.0f);
	glm::vec3 v1(+1.0f,-1.0f,-1.0f);
	glm::vec3 v2(-1.0f,+1.0f,-1.0f);
	glm::vec3 v3(+1.0f,+1.0f,-1.0f);
	glm::vec3 v4(-1.0f,-1.0f,+1.0f);
	glm::vec3 v5(+1.0f,-1.0f,+1.0f);
	glm::vec3 v6(-1.0f,+1.0f,+1.0f);
	glm::vec3 v7(+1.0f,+1.0f,+1.0f);

	std::vector< glm::vec3 > verts{
		v0,v1,v4, v4,v1,v5,
		v0,v2,v1, v1,v2,v3,
		v0,v4,v2, v2,v4,v6,
		v7,v6,v5, v5,v6,v4,
		v7,v5,v3, v3,v5,v1,
		v7,v3,v6, v6,v3,v2
	};

	cube_mesh_count = verts.size();

	//upload verts to GPU:
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//create vertex array object describing layout (position is used as texture coordinate):
	auto Position = MeshBuffer::Attrib(3, GL_FLOAT, MeshBuffer::Attrib::AsFloat, sizeof(glm::vec3), 0);
	auto TexCoord = MeshBuffer::Attrib(3, GL_FLOAT, MeshBuffer::Attrib::AsFloat, sizeof(glm::vec3), 0);

	std::vector< std::pair< char const *, MeshBuffer::Attrib const & > > attribs;
	attribs.emplace_back("Position", Position);
	attribs.emplace_back("TexCoord", TexCoord);

	return new GLuint(make_vao_for_program(vbo, attribs.begin(), attribs.end(), cube_program->program, "cube_program"));
});

Load< std::vector<MeshBuffer *> > meshes(LoadTagDefault, [](){
	std::vector<MeshBuffer *> *ret = new std::vector<MeshBuffer *>;

	ret->push_back(new MeshBuffer(data_path("vignette.pnct"), GL_STATIC_DRAW));
	ret->push_back(new MeshBuffer(data_path("suzanne.pnct"), GL_STATIC_DRAW));
	suzanne_mesh = new MeshBuffer(data_path("face.pnct"), GL_DYNAMIC_DRAW);
	ret->push_back(suzanne_mesh);

	unsigned short idx = 0;
	for(MeshBuffer *cur : *ret) {
		for(std::pair<std::string, MeshBuffer::Mesh> p : cur->meshes) {
			mesh2idx.emplace(p.first, idx);
		}
		++idx; // i dont care about writing a for loop the right way
	}

	return ret;
});


Load< std::vector<GLuint> > meshes_for_texture_program(LoadTagDefault, [](){
	std::cout << "Loading texture program..." << std::flush;
	std::vector<GLuint> *ret = new std::vector<GLuint>;
	for(int x = 0; x < meshes->size(); ++x) {
		ret->push_back((*meshes)[x]->make_vao_for_program(texture_program->program));
	}
	return ret;
});


Load< std::vector<GLuint> > meshes_for_depth_program(LoadTagDefault, [](){
	std::cout << "Loading depth program..." << std::flush;
	std::vector<GLuint> *ret = new std::vector<GLuint>;
	for(int x = 0; x < meshes->size(); ++x) {
		ret->push_back((*meshes)[x]->make_vao_for_program(depth_program->program));
	}
	std::cout << "Done." << std::endl;
	return ret;
});

//used for fullscreen passes:
Load< GLuint > empty_vao(LoadTagDefault, [](){
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindVertexArray(0);
	return new GLuint(vao);
});

MLoad< Font > font_noto_it(LoadTagDefault, [](){
	return new Font("fonts/noto_italic.fnt", glm::vec2(640, 480));
});

MLoad< Font > font_times(LoadTagDefault, [](){
	return new Font("fonts/times.fnt", glm::vec2(640, 480));
});


GLuint load_texture(std::string const &filename) {
	glm::uvec2 size;
	std::vector< glm::u8vec4 > data;
	load_png(filename, &size, &data, LowerLeftOrigin);

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	//// new load texture with alpha based on font
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	GL_ERRORS();

	return tex;
}

Load< GLuint > wood_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/wood.png")));
});

Load< GLuint > marble_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/marble.png")));
});

Load< GLuint > start_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/start.png")));
});

Load< GLuint > logo_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/logo_transparent.png")));
});

Load< GLuint > neutral_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/neutral_face.png")));
});

Load< GLuint > smile_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/happy_face.png")));
});

Load< GLuint > sad_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/sad_face.png")));
});

Load< GLuint > surprise_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/surprise_face.png")));
});

Load< GLuint > confuse_tex(LoadTagDefault, [](){
	return new GLuint(load_texture(data_path("textures/confused_face.png")));
});


Load< GLuint > white_tex(LoadTagDefault, [](){
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glm::u8vec4 white(0xff, 0xff, 0xff, 0xff);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(white));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	return new GLuint(tex);
});


void GameMode::change_game_state() {
	game_state = states[rand() %4];
	if (game_state == HAPPY) {
		middle_text = happy_strs[rand()%4];
	} else if (game_state == SAD) {
		middle_text = sad_strs[rand()%3];
	} else if (game_state == CONFUSED) {
		middle_text = conf_strs[rand()%3];
	} else if (game_state == SURPRISE) {
		middle_text = supr_strs[rand()%4];
	}
}

Scene::Transform *camera_parent_transform = nullptr;
Scene::Camera *camera = nullptr;
Scene::Transform *spot_parent_transform = nullptr;
Scene::Lamp *spot = nullptr;

Scene::Object *face_object = nullptr;
Scene::Object *suzanne_object = nullptr;

Load< Scene > scene(LoadTagDefault, [](){
	Scene *ret = new Scene;

	//pre-build some program info (material) blocks to assign to each object:
	Scene::Object::ProgramInfo texture_program_info;
	texture_program_info.program = texture_program->program;
	texture_program_info.vao = 0; // will set later
	texture_program_info.mvp_mat4  = texture_program->object_to_clip_mat4;
	texture_program_info.mv_mat4x3 = texture_program->object_to_light_mat4x3;
	texture_program_info.itmv_mat3 = texture_program->normal_to_light_mat3;
	texture_program_info.wtcl_mat4 = texture_program->world_to_clip_mat4;

	Scene::Object::ProgramInfo depth_program_info;
	depth_program_info.program = depth_program->program;
	depth_program_info.vao = 0; // will set later
	depth_program_info.mvp_mat4  = depth_program->object_to_clip_mat4;

	// Note: rendered in reverse-loading order
	std::vector<std::string const> names;
	names.push_back(data_path("suzanne.scene"));
	names.push_back(data_path("vignette.scene"));
	names.push_back(data_path("face.scene"));

	//load transform hierarchy:
	ret->load(names, [&](Scene &s, Scene::Transform *t, std::string const *m_){
		Scene::Object *obj = s.new_object(t);

		if(m_ == nullptr)
			return;

		std::string const &m = *m_;

		obj->programs[Scene::Object::ProgramTypeDefault] = texture_program_info;
		if (t->name == "Platform") {
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *wood_tex;
		} else if (t->name == "Pedestal") {
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *marble_tex;
		} else if (t->name == "face") {
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *white_tex;
			obj->programs[Scene::Object::ProgramTypeDefault].textures[2] = *diffuse_cube;
			obj->programs[Scene::Object::ProgramTypeDefault].texture_type[2] = GL_TEXTURE_CUBE_MAP;
			face_object = obj;
		} else if (t->name == "suzanne") {
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *white_tex;
			obj->programs[Scene::Object::ProgramTypeDefault].zwrite = false;
			obj->programs[Scene::Object::ProgramTypeShadow].zwrite = false;
			suzanne_object = obj;
		} else {
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *white_tex;
		}

		obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

		MeshBuffer::Mesh const *mesh;
		for(const MeshBuffer *const cur : (*meshes)) {
			if(cur->contains(m)) {
				mesh = &cur->lookup(m);
			}
		}

		obj->programs[Scene::Object::ProgramTypeDefault].vao = (*meshes_for_texture_program)[mesh2idx[m]];
		obj->programs[Scene::Object::ProgramTypeShadow].vao = (*meshes_for_depth_program)[mesh2idx[m]];

		obj->programs[Scene::Object::ProgramTypeDefault].start = mesh->start;
		obj->programs[Scene::Object::ProgramTypeDefault].count = mesh->count;

		obj->programs[Scene::Object::ProgramTypeShadow].start = mesh->start;
		obj->programs[Scene::Object::ProgramTypeShadow].count = mesh->count;
	});

	//look up camera parent transform:
	for (Scene::Transform *t = ret->first_transform; t != nullptr; t = t->alloc_next) {
		if (t->name == "CameraParent") {
			if (camera_parent_transform) throw std::runtime_error("Multiple 'CameraParent' transforms in scene.");
			camera_parent_transform = t;
		}
		if (t->name == "SpotParent") {
			if (spot_parent_transform) throw std::runtime_error("Multiple 'SpotParent' transforms in scene.");
			spot_parent_transform = t;
		}

	}
	if (!camera_parent_transform) throw std::runtime_error("No 'CameraParent' transform in scene.");
	if (!spot_parent_transform) throw std::runtime_error("No 'SpotParent' transform in scene.");

	//look up the camera:
	for (Scene::Camera *c = ret->first_camera; c != nullptr; c = c->alloc_next) {
		if (c->transform->name == "Camera") {
			if (camera) throw std::runtime_error("Multiple 'Camera' objects in scene.");
			camera = c;
		}
	}
	if (!camera) throw std::runtime_error("No 'Camera' camera in scene.");

	//look up the spotlight:
	for (Scene::Lamp *l = ret->first_lamp; l != nullptr; l = l->alloc_next) {
		if (l->transform->name == "Spot") {
			if (spot) throw std::runtime_error("Multiple 'Spot' objects in scene.");
			if (l->type != Scene::Lamp::Spot) throw std::runtime_error("Lamp 'Spot' is not a spotlight.");
			spot = l;
		}
	}
	if (!spot) throw std::runtime_error("No 'Spot' spotlight in scene.");

	return ret;

});

UIElement *GameMode::create_prompt(glm::vec2 loc,
		glm::vec2 size,
		GLuint tex, 
		std::string name) {
	UIBox *ret = new UIBox(
		loc,
		size,
		glm::vec4(1,1,1,1.0f));
	ret->tex = tex;
	ret->name = name;

    ret->onResize = [ret](glm::vec2 const& old_size, glm::vec2 const& new_size){
		glm::vec2 old_loc = glm::vec2(ret->pos.x / old_size.x, ret->pos.y / old_size.y);
		glm::vec2 old_hw = glm::vec2(ret->size.x / old_size.x, ret->size.y / old_size.y);
		glm::vec2 loc = glm::vec2(new_size.x*old_loc.x, new_size.y*old_loc.y);
		glm::vec2 size = glm::vec2(new_size.x*old_hw.x, new_size.y*old_hw.y);
		ret->pos = loc;
		ret->size = size;
	};
	return ret;
}

UIElement *GameMode::create_button(glm::vec2 loc,
		glm::vec2 size,
		GLuint tex, 
		std::string name) {
	UIBox *ret = new UIBox(
		loc,
		size,
		glm::vec4(1,1,1,1.0f));
	ret->tex = tex;
	ret->name = name;
	ret->onMouseDown = [ret](glm::vec2 const& m, bool i, int b) {
        if(i) UIElement::ui_element_focused = (UIElement *)ret;
        std::cout << "start down" << std::endl;
        game_state = HAPPY;
    };

    ret->onMouseUp = [ret](glm::vec2 const& m, bool i, int b) {
        if((UIElement *)ret == UIElement::ui_element_focused) 
            UIElement::ui_element_focused = nullptr;
    };

    ret->onResize = [ret](glm::vec2 const& old_size, glm::vec2 const& new_size){
		glm::vec2 old_loc = glm::vec2(ret->pos.x / old_size.x, ret->pos.y / old_size.y);
		glm::vec2 old_hw = glm::vec2(ret->size.x / old_size.x, ret->size.y / old_size.y);
		glm::vec2 loc = glm::vec2(new_size.x*old_loc.x, new_size.y*old_loc.y);
		glm::vec2 size = glm::vec2(new_size.x*old_hw.x, new_size.x*old_hw.x);
		ret->pos = loc;
		ret->size = size;
	};
	return ret;
}

static size_t get_closest_weight(std::vector<float> weights, ShapeKeyMesh *const face) {
	const std::vector<std::string> wnames = {
		"brow_up_L",
		"brow_up_R",
		"brow_down_L",
		"brow_down_R",
		"brow_out_L",
		"brow_out_R",
		"brow_in_L",
		"brow_in_R",
		"cheek_up_L",
		"cheek_up_R",
		"cheek_out_L",
		"cheek_out_R",
		"cheek_down_L",
		"cheek_down_R",
		"cheek_in_L",
		"cheek_in_R",
		"chin_down",
		"chin_up",
		"chin_right",
		"chin_left"
	};
	std::vector<float> happy = {
		0.4f,0.4f,0.f,0.f,0.2f,0.2f,0.f,0.f,0.8f,0.8f,0.f,0.f,0.f,0.f,0.f,0.f,0.4f,0.f,0.f,0.f
	};
	std::vector<float> sad = {
		0.f,0.f,0.8f,0.8f,0.f,0.f,0.4f,0.4f,0.f,0.f,0.f,0.f,0.8f,0.8f,0.f,0.f,0.f,0.f,0.f,0.f
	};
	std::vector<float> confused_L = {
		0.7f,0.f,0.f,0.5f,0.f,0.f,0.4f,0.2f,0.f,0.f,0.f,0.f,0.5f,0.5f,0.f,0.f,0.4f,0.f,0.f,0.f
	};
	std::vector<float> confused_R = {
		0.5f,0.f,0.f,0.7f,0.f,0.f,0.4f,0.2f,0.f,0.f,0.f,0.f,0.5f,0.5f,0.f,0.f,0.4f,0.f,0.f,0.f
	};
	std::vector<float> surprised = {
		0.8f,0.8f,0.f,0.f,0.4f,0.4f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.f,0.8f,0.8f,0.5f,0.f,0.f,0.f
	};
	if(weights.size() != happy.size()) {
		std::stringstream errs;
		errs << "wrong weights size given, got " << weights.size() << " expected " << happy.size();
		throw std::runtime_error(errs.str());
	}

	float whappy = 0, wsad = 0, wconfusedL = 0, wconfusedR = 0, wsurprised = 0, wweights = 0;
	for(int x = 0; x < weights.size(); ++x) {
		whappy += happy[x];
		wsad += sad[x];
		wconfusedL += confused_L[x];
		wconfusedR += confused_R[x];
		wsurprised += surprised[x];
		wweights += weights[x];
	}

	//normalize
	for(int x = 0; x < weights.size(); ++x) {
		happy[x] /= whappy;
		sad[x] /= wsad;
		confused_L[x] /= wconfusedL;
		confused_R[x] /= wconfusedR;
		surprised[x] /= wsurprised;
		if(wweights > 0.001f) weights[x] /= wweights;
	}

	whappy = wsad = wconfusedL = wconfusedR = wsurprised = 0.f;
	for(int x = 0; x < weights.size(); ++x) {
		size_t oidx = face->frame_map[wnames[x]].index;
		whappy += happy[x] * weights[oidx];
		wsad += sad[x] * weights[oidx];
		wconfusedL += confused_L[x] * weights[oidx];
		wconfusedR += confused_R[x] * weights[oidx];
		wsurprised += surprised[x] * weights[oidx];
	}

	/*
	std::cout << "whappy: " << whappy << std::endl
			  << "wsad:   " << wsad << std::endl
			  << "wconfL: " << wconfusedL << std::endl
			  << "wconfR: " << wconfusedR << std::endl
			  << "wsurpr: " << wsurprised << std::endl;
	*/

	if(whappy < 0.1f && wsad < 0.1f && wconfusedL < 0.1f && wconfusedR < 0.1f && wsurprised < 0.1f)
		return NEUTRAL;

	if(whappy >= wsad && whappy >= wconfusedL && whappy >= wconfusedR && whappy >= wsurprised)
		return HAPPY;
	else if(wsad >= whappy && wsad >= wconfusedL && wsad >= wconfusedR && wsad >= wsurprised)
		return SAD;
	else if((wconfusedL >= whappy && wconfusedL >= wsad && wconfusedL >= wconfusedR && wconfusedL >= wsurprised) ||
			(wconfusedR >= whappy && wconfusedR >= wsad && wconfusedR >= wconfusedL && wconfusedR >= wsurprised))
		return CONFUSED;
	else if(wsurprised >= whappy && wsurprised >= wconfusedL && wsurprised >= wconfusedR && wsurprised >= wsad)
		return SURPRISE;

	return NEUTRAL;
}

/* Intersects the rays A and B, and returns the point of intersection.
 * If A and B do not intersect, returns the closest point on A to B.
 * max_distance is the maximum distance from Aogn, along Adir, that
 * may be returned.
 */
glm::vec3 soft_ray_isect(
	glm::vec3 const& Aogn, glm::vec3 const& Adir, 
	glm::vec3 const& Bogn, glm::vec3 const& Bdir,
	float max_distance = std::numeric_limits<float>::infinity()) {
	// ATTRIBUTION: Closest point between two rays
	// http://morroworks.palitri.com/Content/Docs/Rays%20closest%20point.pdf
	glm::vec3 const& a = Adir;
	glm::vec3 const& b = Bdir;
	glm::vec3 c = Bogn - Aogn;

	float dist2 = glm::dot(c, c);
	float a_b = glm::dot(a, b);
	float b_b = glm::dot(b, b);
	float a_a = glm::dot(a, a);

	if(abs(a_b * a_b - a_a * b_b) < 0.001f || a_a < 0.001f || b_b < 0.001f) {
		// This technique throws out NaNs if a and b are close
		// to colinear (this is a limitation discussed in the linked paper)
		//std::cout << "(no true isect)" << std::endl;
		glm::vec3 r = Bogn - Aogn;
		return Aogn + std::max(0.f, glm::dot(Adir, r)) * Adir;
	}

	c /= glm::sqrt(dist2);

	float b_c = glm::dot(b, c);
	float a_c = glm::dot(a, c);

	/* // Uncomment for debugging //
	std::cout << "--------------------" << std::endl
			  << "Aogn: " << glm::to_string(Aogn) << std::endl
			  << "Bogn: " << glm::to_string(Bogn) << std::endl
			  << "Adir: " << glm::to_string(Adir) << std::endl
			  << "Bdir: " << glm::to_string(Bdir) << std::endl
			  << "denm: " << (a_a * b_b - a_b * a_b) << std::endl
			  << "a_a:  " << a_a << std::endl
			  << "b_b:  " << b_b << std::endl;
	*/

	float d = -(a_b * b_c + a_c * b_b) / (a_a * b_b - a_b * a_b);
	return Aogn + Adir * std::max(0.f, d);
}

UIElement *GameMode::create_shapekey_deformer(
	float size_scr,
	ShapeKeyMesh *mesh,
	uint32_t vertex,
	std::function<void(uint32_t/*vertex*/, glm::vec3/*pos*/, std::vector<float>/*weights*/)> value_changed,
	std::function<glm::mat4()> mesh2world, std::function<glm::mat4()> world2mesh) {

	auto mesh_to_uispace = [](glm::vec3 const& pos, glm::mat4 const& m2w, glm::vec2 const& ws){
		glm::vec2 clp = camera->world_to_clip(m2w * glm::vec4(pos, 1.f));
		glm::vec2 scr = clp/2.f + glm::vec2(.5f,.5f);

		scr.x *= ws.x;
		scr.y *= ws.y;
		return scr;
	};
	
	UIBox *ret = new UIBox(
		mesh_to_uispace(mesh->get_vertex(vertex), mesh2world(), window_size),
		glm::vec2(size_scr,size_scr),
		glm::vec4(1,1,1,0.7f));

	ret->onMouseDown = [ret](glm::vec2 const& m, bool i, int b) {
        if(i) UIElement::ui_element_focused = (UIElement *)ret;
    };

    ret->onMouseUp = [ret](glm::vec2 const& m, bool i, int b) {
        if((UIElement *)ret == UIElement::ui_element_focused) 
            UIElement::ui_element_focused = nullptr;
    };

	ret->onHover = [ret,vertex,value_changed,mesh,mesh_to_uispace,mesh2world,world2mesh,this](glm::vec2 const& m_, bool i) {
		glm::vec3 cur = mesh->get_vertex(vertex);
		glm::mat4 m2w = mesh2world();
		ret->pos = mesh_to_uispace(cur, m2w, window_size);

		glm::vec2 m = glm::vec2(m_.x / window_size.x, m_.y / window_size.y);

		if((UIElement *)ret == UIElement::ui_element_focused) {
			std::vector<float> nweights; nweights.resize(weights.size(), 0);

			glm::mat4 w2m = world2mesh();
			glm::mat4 camera_l2w = camera->transform->make_local_to_world();

			glm::vec4 cam4 = w2m * camera_l2w * glm::vec4(0.f, 0.f, 0.f, 1.f);
			cam4 /= cam4.w;
			glm::vec3 cam3 = glm::vec3(cam4);

			glm::vec2 const mm = m * 2.f - 1.f;
			glm::vec4 const oray = glm::vec4(camera->generate_ray(mm), 0.f);
			glm::vec4 const ray4 = w2m * camera_l2w * oray;
			glm::vec3 const ray = glm::normalize(ray4);

			/* // Uncomment for coordinate space debuging //
			std::cout << "-----------" << std::endl 
					<< "mm:   " << glm::to_string(mm) << std::endl 
					<< "rlen: " << glm::length(ray) << std::endl 
					<< "orlen:" << glm::length(oray) << std::endl 
					<< "dist: " << dist << std::endl 
					<< "ray4: " << glm::to_string(ray4) << std::endl 
					<< "ray:  " << glm::to_string(ray) << std::endl 
					<< "cam3: " << glm::to_string(cam3) << std::endl 
					<< "cur:  " << glm::to_string(cur) << std::endl 
					<< "tar:  " << glm::to_string(target) << std::endl 
					<< "rem:  " << glm::to_string(rem) << std::endl;
			*/

			glm::vec3 const ref = mesh->vertex_buf[mesh->reference_key.start_vertex + vertex];
			glm::vec3 cur_proj = ref;

			// Gram schmidt
			for(int x = 0; x < mesh->key_frames.size(); ++x) {
				// Get range of movement for this key
				glm::vec3 const key = mesh->vertex_buf[mesh->key_frames[x].start_vertex + vertex];
				glm::vec3 const ref2key = key - ref;
				glm::vec3 const cur2key = key - cur_proj;

				// Project range of movement on the target
				float d2_cur2key = glm::dot(cur2key, cur2key);
				float d2_ref2key = glm::dot(ref2key, ref2key);

				if(d2_cur2key > 0.001f && d2_ref2key > 0.001f) {
					// Instead of subtracting the projection directly like normal gram-schmidt,
					// we reproject the current point onto the camera ray to find the closest point
					// on the ray.  We do this because we don't actually want the target vertex to
					// move to any previously-calculated target pos; we instead want the target vertex
					// to move to the camera ray.  The closest point on the camera ray changes as
					// gram-schmidt proceeds, so we need to readjust.
					// 
					// First, we find the closest point on the shape key vector (ref2key) to the camera
					// ray, clamping to the shapekey's range of movement.  Then we project this point back 
					// onto the camera ray to avoid biasing to irrelevant values (ex, an eyebrow deformer
					// pulling the mouth to 1.0 weight because the target point happens to be on the mouth
					// key's ray).
					float d_ref2key = glm::sqrt(d2_ref2key);
					glm::vec3 target = soft_ray_isect(cur_proj, ref2key / d_ref2key, cam3, ray, d_ref2key);
					glm::vec3 r = target - cam3;
					target = cam3 + std::max(0.f, glm::dot(ray, r)) * ray;

					/* // uncomment for debugging //
					std::cout << "key: " << mesh->key_frames[x].name << std::endl
							  << "cur: " << glm::to_string(cur_proj) << std::endl
							  << "tar: " << glm::to_string(target) << std::endl
							  << "r2k: " << glm::to_string(ref2key) << std::endl << std::endl;
					*/
					glm::vec3 cur2tar = target - cur_proj;

					float a = glm::dot(cur2tar, ref2key) / glm::dot(ref2key, ref2key);
					a = std::max(0.f, std::min(1.f, a));

					glm::vec3 proj = a * ref2key;
					// "Subtract" projection from remainder & set weight
					cur_proj += proj;
					
					nweights[x] = a;
				}
				else
					nweights[x] = 0;
			}

            if(value_changed)
                value_changed(vertex, cur_proj, nweights);

            ret->pos = mesh_to_uispace(mesh->get_vertex(vertex), m2w, window_size);
        }
	};

	ret->onResize = [mesh2world, vertex, mesh, ret, mesh_to_uispace](glm::vec2 const& old_size, glm::vec2 const& new_size){
		glm::vec3 cur = mesh->get_vertex(vertex);
		glm::mat4 m2w = mesh2world();
		ret->pos = mesh_to_uispace(cur, m2w, new_size);
	};

	return ret;
}

const size_t num_deformers = 5;
static std::array<std::vector<float>, num_deformers> deformer_weights;
static std::array<size_t, num_deformers> reference_vertices;

GameMode::GameMode(glm::uvec2 const& window_size) {
	srand(time(0));
	face = new ShapeKeyMesh("face.keys", suzanne_mesh); //TODO remember to destroy
	
	//CHANGE middle_text WHEN WE CHANGE GAME STATES
	middle_text = happy_strs[rand()%3];
	// ****

	SDL_Event fake;
	fake.type = SDL_WINDOWEVENT_RESIZED;

	weights.resize(face->key_frames.size());
	float itl_ypos = window_size.y / 2.f + 25.f * weights.size() / 2.f;
	for(int x = 0; x < weights.size(); ++x) {
		weights[x] = 0.f;
		ui_elements.push_back(
			UIElement::create_slider(
				glm::vec2(200.f,itl_ypos - x*25), 
				100, 10, 20, 
				[x, this](float n){weights[x]=n;}, 
				[x, this](){return weights[x];},
				weights[x],
				Left,
				Middle
				)
			);
		ui_elements[x]->handle_event(fake, window_size);
		ui_elements[x]->enabled = false;
		ui_elements[x]->name = "debug_slider";
	}

	reference_vertices = {
		40, 250, 3565, 6277, 2932
	};

	
	for(int x = 0; x < num_deformers; ++x) {
		deformer_weights[x].resize(weights.size());
		for(int y = 0; y < weights.size(); ++y)
			deformer_weights[x][y] = 0.f;
	}

	for(int x = 0; x < num_deformers; ++x) {
		UIElement *deformer = create_shapekey_deformer(20, face, reference_vertices[x], 
			[this, x](uint32_t vert, glm::vec3 pos, std::vector<float> ws){
				deformer_weights[x] = ws;
				for(int i = 0; i < weights.size(); ++i) {
					weights[i] = 0;
					for(int j = 0; j < num_deformers; ++j) {
						if(::reference_vertices[j] == 40 && face->key_frames[i].name.find("cheek_") == 0)
							continue;

						weights[i] += deformer_weights[j][i];
					}
					weights[i] = std::min(1.f, weights[i]);
				}
			},
			std::bind(&Scene::Transform::make_local_to_world, face_object->transform), 
			std::bind(&Scene::Transform::make_world_to_local, face_object->transform)
		);

		deformer->name = "deformer";
		ui_elements.push_back(deformer);
	}

	UIBox *text_bg = new UIBox(
		glm::vec2(window_size.x / 2.f, window_size.y * .2f),
		glm::vec2(window_size.x, 50.f),
		glm::vec4(0,0,0,.5f));
	text_bg->onResize = [text_bg](glm::vec2 const& old, glm::vec2 const& nxt) {
		text_bg->pos.x = nxt.x / 2.f;
		text_bg->pos.y = nxt.y * .2f;
		text_bg->size.x = nxt.x;
	};
	text_bg->name = "text_bg";
	ui_elements.push_back(text_bg);

	/* Emotion prompt */
	glm::vec2 loc = glm::vec2(window_size.x/2.0f, window_size.y*2.0f/3.0f);
	glm::vec2 size = glm::vec2(window_size.x/2.5f, window_size.x/2.5f);

	UIElement *logo = create_prompt(loc, size, *logo_tex, "logo");
	ui_elements.push_back(logo);

	loc.y = window_size.y/6.0f;
	size = glm::vec2(window_size.x/4.0f, window_size.x/4.0f);
	UIElement *start = create_button(loc, size, *start_tex, "start");
	ui_elements.push_back(start);

	{
		loc = glm::vec2(3*window_size.x/4.0f, 3*window_size.y/4.0f);
		size = glm::vec2(window_size.x/8.0f, window_size.x/8.0f);
		UIElement *neutral = create_prompt(loc, size, *neutral_tex, "neutral");
		neutral->enabled = false;
		UIElement *happy = create_prompt(loc, size, *smile_tex, "happy");
		happy->enabled = false;
		UIElement *sad = create_prompt(loc, size, *sad_tex, "sad");
		sad->enabled = false;
		UIElement *confuse = create_prompt(loc, size, *confuse_tex, "confuse");
		confuse->enabled = false;
		UIElement *surprise = create_prompt(loc, size, *surprise_tex, "surprise");
		surprise->enabled = false;
		ui_elements.push_back(neutral);
		ui_elements.push_back(happy);
		ui_elements.push_back(sad);
		ui_elements.push_back(confuse);
		ui_elements.push_back(surprise);
	}

	this->window_size = window_size;
}

GameMode::~GameMode() {
}

bool GameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	this->window_size = window_size;

	SDL_Event e = evt;
	switch(e.type) {
        case SDL_MOUSEMOTION:
        e.motion.y = window_size.y - e.motion.y;

        cur_mouse_pos = glm::vec2(e.motion.x, e.motion.y);
        break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        e.button.y = window_size.y - e.button.y;
        break;//debug_slider
        case SDL_KEYDOWN:
        switch(evt.key.keysym.sym) {
        	case SDLK_F3:
        	if(SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) {
        		debug_mode_enabled = !debug_mode_enabled;

        		SDL_Event fake;
        		fake.type = SDL_WINDOWEVENT_RESIZED;

        		for(UIElement *cur : ui_elements) {
        			if(cur->name == "debug_slider") {
        				cur->enabled = debug_mode_enabled;
        				cur->handle_event(fake, window_size);
        			}
        		}
        	}
        	break;
        }
        break;
    }

    if(e.type == SDL_MOUSEMOTION 
    	&& UIElement::ui_element_focused 
    	&& UIElement::ui_element_focused->name == "deformer") {
    	for(int x = 0; x < weights.size(); ++x)
			weights[x] = 0;
    }

	for(UIElement *cur : ui_elements) {
		if(cur->enabled) {
			cur->handle_event(e, window_size);
		}

	}

	return false;
}

static size_t current_expression = NEUTRAL;

void GameMode::update(float elapsed) {
	//camera_parent_transform->rotation = glm::angleAxis(camera_spin, glm::vec3(0.0f, 0.0f, 1.0f));
	spot_parent_transform->rotation = glm::angleAxis(spot_spin, glm::vec3(0.0f, 0.0f, 1.0f));

	timer += elapsed;

	if (timer >= 10.0f) {
		timer = 0.0f;
		change_game_state();
	}
	// std::cout << game_state << std::endl;
	for(UIElement *cur : ui_elements) {
		if (cur->name== "logo") cur->enabled = game_state == MENU;
		if (cur->name== "start") cur->enabled = game_state == MENU;
		if (cur->name== "neutral") cur->enabled = game_state == NEUTRAL;
		if (cur->name== "happy") cur->enabled = game_state == HAPPY;
		if (cur->name== "sad") cur->enabled = game_state == SAD;
		if (cur->name== "confuse") cur->enabled = game_state == CONFUSED;
		if (cur->name== "surprise") cur->enabled = game_state == SURPRISE;
		if (cur->name== "text_bg") cur->enabled = game_state != MENU;
		if (cur->name== "deformer") cur->enabled = game_state != MENU;

		if(cur->enabled) 
			cur->update(elapsed);
	}

	face_object->transform->position.z = 1;
	face_object->transform->scale = glm::vec3(0.5f,0.5f,0.5f);
	face->recalculate_mesh_data(weights);

	current_expression = get_closest_weight(weights, face);

	
	for(int i = 0; i < weights.size(); ++i) {
		for(int y = 0; y < deformer_weights.size(); ++y) {
			deformer_weights[y][i] -= elapsed * 0.15f;
			deformer_weights[y][i] = std::max(deformer_weights[y][i], 0.f);
		}

		weights[i] = 0;
		for(int j = 0; j < deformer_weights.size(); ++j) {
			if(reference_vertices[j] == 40 && face->key_frames[i].name.find("cheek_") == 0)
				continue;

			weights[i] += deformer_weights[j][i];
		}
		weights[i] = std::min(1.f, weights[i]);
	}

    //TODO
    /** state logic **/
    if (state == WAIT_FOR_INPUT) {
        //display text
        if (!did_happy) {
            happy = true;
            sad = false;
            gg = false;
        } else {
            happy = false;
            sad = true;
            gg = false;
        }
        if (posedge_clock == 1) {
            //start timer
            t0 = Time::now();
            posedge_clock = 0;
        } else {
            t1 = Time::now();
            fsec fs = t1 - t0;
            if (fs.count() >= 7) {
                state = CHECK;
                posedge_clock = 1;
            }
        }
    } else if (state == CHECK) {
        happy = false;
        sad = false;
        gg = true;
        did_happy = !did_happy;
        //check if theyre right, if they do display text
        //TODO erm actually do something if input is wrong
        //if wrong go to game over
        if (posedge_clock == 1) {
            //start timer
            t0 = Time::now();
            posedge_clock = 0;
        } else {
            t1 = Time::now();
            fsec fs = t1 - t0;
            if (fs.count() >= 3) {
                state = WAIT_FOR_INPUT;
                posedge_clock = 1;
            }
        }

    } else if (state == GAME_OVER) {

    }
}

//GameMode will render to some offscreen framebuffer(s).
//This code allocates and resizes them as needed:
struct Framebuffers {
	glm::uvec2 size = glm::uvec2(0,0); //remember the size of the framebuffer

	//This framebuffer is used for fullscreen effects:
	GLuint color_tex = 0;
	GLuint depth_rb = 0;
	GLuint fb = 0;

	//This framebuffer is used for shadow maps:
	glm::uvec2 shadow_size = glm::uvec2(0,0);
	GLuint shadow_color_tex = 0; //DEBUG
	GLuint shadow_depth_tex = 0;
	GLuint shadow_fb = 0;

	/* Returns the z-depth for the given pixel by sampling the depth buffer.
	 * Returns a value from 0 to 1, with 0 being the near clip plane and 1 is the far plane
	 * note: scr is in pixels, not normalized screen space
	 */
	float sample_depth(glm::uvec2 const &scr) {
		float ret;
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		glReadPixels(scr.x, scr.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &ret);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return ret;
	}

	void allocate(glm::uvec2 const &new_size, glm::uvec2 const &new_shadow_size) {
		//allocate full-screen framebuffer:
		if (size != new_size) {
			size = new_size;

			if (color_tex == 0) glGenTextures(1, &color_tex);
			glBindTexture(GL_TEXTURE_2D, color_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);

			if (depth_rb == 0) glGenRenderbuffers(1, &depth_rb);
			glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size.x, size.y);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			if (fb == 0) glGenFramebuffers(1, &fb);
			glBindFramebuffer(GL_FRAMEBUFFER, fb);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
			check_fb();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			GL_ERRORS();
		}

		//allocate shadow map framebuffer:
		if (shadow_size != new_shadow_size) {
			shadow_size = new_shadow_size;

			if (shadow_color_tex == 0) glGenTextures(1, &shadow_color_tex);
			glBindTexture(GL_TEXTURE_2D, shadow_color_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, shadow_size.x, shadow_size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);


			if (shadow_depth_tex == 0) glGenTextures(1, &shadow_depth_tex);
			glBindTexture(GL_TEXTURE_2D, shadow_depth_tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadow_size.x, shadow_size.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);

			if (shadow_fb == 0) glGenFramebuffers(1, &shadow_fb);
			glBindFramebuffer(GL_FRAMEBUFFER, shadow_fb);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadow_color_tex, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_depth_tex, 0);
			check_fb();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			GL_ERRORS();
		}
	}
} fbs;



void GameMode::draw(glm::uvec2 const &drawable_size) {
	fbs.allocate(drawable_size, glm::uvec2(2048, 2048));

	//Draw scene to shadow map for spotlight:
	glBindFramebuffer(GL_FRAMEBUFFER, fbs.shadow_fb);
	glViewport(0,0,fbs.shadow_size.x, fbs.shadow_size.y);

	glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	//render only back faces to shadow map (prevent shadow speckles on fronts of objects):
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);

	scene->draw(spot, Scene::Object::ProgramTypeShadow);

	glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERRORS();

	//Draw scene to off-screen framebuffer:
	glBindFramebuffer(GL_FRAMEBUFFER, fbs.fb);
	glViewport(0,0,drawable_size.x, drawable_size.y);

	camera->aspect = drawable_size.x / float(drawable_size.y);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set up basic OpenGL state:
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//set up light positions:
	glUseProgram(texture_program->program);

	//don't use distant directional light at all (color == 0):
	// glUniform3fv(texture_program->sun_color_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));
	// glUniform3fv(texture_program->sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(0.0f, 0.0f,-1.0f))));
	// //use hemisphere light for subtle ambient light:
	// glUniform3fv(texture_program->sky_color_vec3, 1, glm::value_ptr(glm::vec3(0.2f, 0.2f, 0.3f)));
	// glUniform3fv(texture_program->sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 1.0f)));

	glm::mat4 world_to_spot =
		//This matrix converts from the spotlight's clip space ([-1,1]^3) into depth map texture coordinates ([0,1]^2) and depth map Z values ([0,1]):
		glm::mat4(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.5f, 0.5f, 0.5f+0.00001f /* <-- bias */, 1.0f
		)
		//this is the world-to-clip matrix used when rendering the shadow map:
		* spot->make_projection() * spot->transform->make_world_to_local();

	glUniformMatrix4fv(texture_program->light_to_spot_mat4, 1, GL_FALSE, glm::value_ptr(world_to_spot));

	glm::mat4 spot_to_world = spot->transform->make_local_to_world();
	glUniform3fv(texture_program->spot_position_vec3, 1, glm::value_ptr(glm::vec3(spot_to_world[3])));
	glUniform3fv(texture_program->spot_direction_vec3, 1, glm::value_ptr(-glm::vec3(spot_to_world[2])));
	glUniform3fv(texture_program->spot_color_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

	GL_ERRORS();
	// glm::vec2 spot_outer_inner = glm::vec2(std::cos(0.5f * spot->fov), std::cos(0.85f * 0.5f * spot->fov));
	// glUniform2fv(texture_program->spot_outer_inner_vec2, 1, glm::value_ptr(spot_outer_inner));

	//This code binds texture index 1 to the shadow map:
	// (note that this is a bit brittle -- it depends on none of the objects in the scene having a texture of index 1 set in their material data; otherwise scene::draw would unbind this texture):
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fbs.shadow_depth_tex);
	//The shadow_depth_tex must have these parameters set to be used as a sampler2DShadow in the shader:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	//NOTE: however, these are parameters of the texture object, not the binding point, so there is no need to set them *each frame*. I'm doing it here so that you are likely to see that they are being set.
	glActiveTexture(GL_TEXTURE0);

	{ //draw the sky by drawing the cube centered at the camera:
		glDisable(GL_DEPTH_TEST); //don't write to depth buffer
		//only render the back of the cube:
		glDisable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glUseProgram(cube_program->program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, *sky_cube);

		//make a matrix that acts as if the camera is at the origin:
		glm::mat4 world_to_camera = camera->transform->make_world_to_local();
		world_to_camera[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::mat4 rotrr = glm::rotate(glm::mat4(1.f), -45.f, glm::vec3(0.f, 1.f, 0.f));
		world_to_camera = rotrr * world_to_camera;
		glm::mat4 world_to_clip = camera->make_projection() * world_to_camera;
		glm::mat4 object_to_clip = world_to_clip;

		glUniformMatrix4fv(cube_program->object_to_clip_mat4, 1, GL_FALSE, glm::value_ptr(object_to_clip));
		glUniform2f(cube_program->res_vec2, drawable_size.x, drawable_size.y);


		glBindVertexArray(*cube_mesh_for_cube_program);

		glDrawArrays(GL_TRIANGLES, 0, cube_mesh_count);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		//reset state:
		glEnable(GL_DEPTH_TEST);
		//glDisable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
	}
	GL_ERRORS();

	scene->draw(camera);

	GL_ERRORS();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERRORS();

	//Copy scene from color buffer to screen
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbs.fb);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, drawable_size.x, drawable_size.y, 
					  0, 0, drawable_size.x, drawable_size.y,
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERRORS();

	glDisable(GL_DEPTH_TEST);

	font_times->screen_dim = window_size;
	font_noto_it->screen_dim = window_size;

    glm::mat4 id4(1);
    for(UIElement *cur : ui_elements) {
    	if(cur->enabled)
			cur->draw(window_size, id4);
	}

	if(game_state != MENU) {
		glm::vec2 size = font_noto_it->string_dims(middle_text.c_str(), 16, 0.8);
		font_noto_it->draw_ascii_string(middle_text.c_str(), glm::vec2(.5f - size.x / 2.f, .2f + size.y / 2.f - 8.f/window_size.y), 16, 0.8);

		{//if(debug_mode_enabled) {
			std::string expression_text = "Expression: ";
			auto expr2str = [](size_t expr) {
				switch(expr) {
					case HAPPY:
					return "HAPPY";
					break;
					case SAD:
					return "SAD";
					break;
					case CONFUSED:
					return "CONFUSED";
					break;
					case SURPRISE:
					return "SURPRISED";
					break;
					default:
					return "NEUTRAL";
					break;
				}
			};
			expression_text += std::string(expr2str(current_expression));
			expression_text += "\nTry to look " + std::string(expr2str(game_state));

			size = font_noto_it->string_dims(expression_text.c_str(), 16, 0.8);
			font_noto_it->draw_ascii_string(expression_text.c_str(), glm::vec2(.5f - size.x / 2.f, .9f + size.y / 2.f - 8.f/window_size.y), 16, 0.8);
		}
	}

	if(debug_mode_enabled) {
		float ypos = window_size.y/2.f + (weights.size() * 25.f)/2.f;
		for(int x = 0; x < weights.size(); ++x) {
			font_times->draw_ascii_string(face->key_frames[x].name.c_str(), glm::vec2(10.f/window_size.x, ypos/window_size.y), 16, 0.8f);
			ypos -= 25.f;
		}

		glm::vec2 smpos = cur_mouse_pos;
		smpos = smpos * ((float)drawable_size.x / (float)window_size.x);
		float raw_depth = fbs.sample_depth((glm::uvec2)smpos);
		glm::vec4 clip = glm::vec4((float)cur_mouse_pos.x/window_size.x*2.f-1.f, (float)cur_mouse_pos.y/window_size.y*2.f-1.f, raw_depth*2.f-1.f, 1.f);
		glm::vec4 view = glm::inverse(camera->make_projection()) * clip;
		view /= view.w;
		glm::vec4 look_ = camera->transform->make_local_to_world() * view;
		glm::vec3 look = glm::vec3(look_.x, look_.y, look_.z);
	
		uint32_t min_vert;
		glm::vec3 min_pos;
		float min_d2 = 1e9;
		glm::mat4 face2w = face_object->transform->make_local_to_world();
		for(uint32_t x = 0; x < face->get_vertex_count(); ++x) {
			glm::vec4 cur_ = face2w * glm::vec4(face->get_vertex(x), 1.f);
			glm::vec3 cur = glm::vec3(cur_.x, cur_.y, cur_.z);
			glm::vec3 dif = look - cur;
			float d2 = glm::dot(dif,dif);
			if(d2 < min_d2) {
				min_vert = x;
				min_d2 = d2;
				min_pos = cur;
			}
		}
		std::stringstream vertss;
		vertss << "Pointed Vertex: " << min_vert << " [" << min_pos.x << ", " << min_pos.y << ", " << min_pos.z << "]";
		font_times->draw_ascii_string(vertss.str().c_str(), glm::vec2(10.f/window_size.x, 1.f-25.f/window_size.y), 16);

		// for debugging
		//suzanne_object->transform->position = look;
	}
	suzanne_object->transform->scale = glm::vec3(0,0,0);//glm::vec3(0.2f, 0.2f, 0.2f);

	glEnable(GL_DEPTH_TEST);
}

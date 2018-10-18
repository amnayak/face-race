#include "Face.hpp"
#include "read_chunk.hpp"
#include "data_path.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <set>
#include <cstddef>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "gl_errors.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "vertex_color_program.hpp"
#include <glm/gtc/matrix_transform.hpp>



static bool DEBUG = true;
static std::string filename = "face.pnct";
static std::string sk_filename = "face_sk.p";

Face::Face() {
  Face::load_shape_key_data();
  //load in the base mesh into the buffer

  //Step 1: Read in file
  static_assert(sizeof(Vertex) == 3*4+3*4+4*1+2*4, "Vertex is packed.");

  std::ifstream file(data_path("face.pnct"), std::ios::binary);

	read_chunk(file, "pnct", &data_to_write);

  if (DEBUG) printf("Reading from file %s\n", filename.c_str());

  //Step 2: upload data (note that it's GL_DYNAMIC_DRAW)
  glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data_to_write.size() * sizeof(Vertex), data_to_write.data(), GL_DYNAMIC_DRAW);
  GL_ERRORS();


  GLuint program = vertex_color_program->program;

  Position = Attrib(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, Position));
  Normal = Attrib(3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, Normal));
  Color = Attrib(4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), offsetof(Vertex, Color));
  TexCoord = Attrib(2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, TexCoord));

  auto bind_attribute = [&](char const *name, Attrib const &attrib) {
  		if (attrib.size == 0) return; //don't bind empty attribs
  		GLint location = glGetAttribLocation(program, name);
  		if (location == -1) {
  			std::cerr << "WARNING: attribute '" << name << "' in mesh buffer isn't active in program." << std::endl;
  		} else {
  			glVertexAttribPointer(location, attrib.size, attrib.type, attrib.normalized, attrib.stride, (GLbyte *)0 + attrib.offset);
  			glEnableVertexAttribArray(location);
  		}
  	};
  GL_ERRORS();

  //Step 3: store attrib locations:
  bind_attribute("Position", Position);
 	bind_attribute("Normal", Normal);
 	bind_attribute("Color", Color);
 	bind_attribute("TexCoord", TexCoord);
  glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

  total = GLuint(data_to_write.size());

  GL_ERRORS();
  if (DEBUG) printf("Uploaded base mesh\n");

}

Face::~Face() {
  //TODO
}

//Loads key frame vertex positions into ShapeKey data structures
void Face::load_shape_key_data() {
  static_assert(sizeof(Face::Vertex) == 3*4+3*4+4*1+2*4, "Vertex is packed.");

  std::ifstream sk_file(data_path(sk_filename), std::ios::binary);

  struct miniVertex {
    glm::vec3 Position;
  };

  std::vector < miniVertex > data;
  std::vector< char > strings;
	read_chunk(sk_file, "p...", &data);
	read_chunk(sk_file, "str0", &strings);

	{ //read index chunk, add to meshes:
		struct IndexEntry {
			uint32_t name_begin, name_end;
			uint32_t vertex_begin, vertex_end;
		};
		static_assert(sizeof(IndexEntry) == 16, "Index entry should be packed");

		std::vector< IndexEntry > index;
		read_chunk(sk_file, "idx0", &index);

		for (auto const &entry : index) {
			std::string name(&strings[0] + entry.name_begin, &strings[0] + entry.name_end);
			Face::ShapeKey mesh;
			mesh.start = entry.vertex_begin;
			mesh.count = entry.vertex_end - entry.vertex_begin;
      for (int i = entry.vertex_begin; i< entry.vertex_end; i++) {
        mesh.verts.push_back(data[i].Position);
        if (DEBUG) printf("%s\n", glm::to_string(data[i].Position).c_str());
      }

			bool inserted = frame_map.insert(std::make_pair(name, mesh)).second;
      key_frames.push_back(mesh); //add to list of frames
      if (name == "Basis") {
        weights.push_back(1.0f); //add a new weight value
      } else {
        weights.push_back(0.0f); //add a new weight value
      }

      if (DEBUG) printf("%lu vertices, inserted key frame %s\n", mesh.verts.size(), name.c_str());
			if (!inserted) {
				std::cerr << "WARNING: mesh name '" + name + "' in filename '" + filename + "' collides with existing mesh." << std::endl;
			}
		}
	}

}

//recalculates the weighted linear combination of the vertices
// and uploads to GPU
void Face::recalculate_mesh_data () {

    //store new linear combination to data_to_write
    for (int v = 0; v < data_to_write.size(); v++) {
      data_to_write[v].Position = glm::vec3(0.f, 0.f, 0.f);
      float weight_sum = 0.0f;
      for(int i = 0; i < key_frames.size(); i++){
        data_to_write[v].Position += key_frames[i].verts[i] * weights[i];
        weight_sum += weights[i];
      }
      assert (weight_sum == 1.0f);
      //TODO: needs a way to ensure that they sum to 1.0..
            //some sort of normalizing fn?
    }

    //Upload vertex data
    assert(key_frames.size() == weights.size());
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
      GL_ARRAY_BUFFER,
      data_to_write.size() * sizeof(Vertex),
      data_to_write.data(),
      GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    cur_vao_size = data_to_write.size();

}

void Face::draw_face(Scene::Transform &t, glm::mat4 const &world_to_clip) {
    glm::mat4 local_to_world = t.make_local_to_world();
  	glm::mat4 mvp = world_to_clip * local_to_world;

    glUseProgram(vertex_color_program->program);
    if(vertex_color_program->object_to_clip_mat4 != 1U){
		    glUniformMatrix4fv(vertex_color_program->object_to_clip_mat4, 1,
                           GL_FALSE, glm::value_ptr(mvp));
    }

    glBindVertexArray(vao);
	  glDrawArrays(GL_TRIANGLES, 0, cur_vao_size * 7);

}

void Face::draw_face(Scene::Transform &t, Scene::Camera const *camera)  {
  glm::mat4 world_to_camera = camera->transform->make_world_to_local();
  glm::mat4 world_to_clip = camera->make_projection() * world_to_camera;
  draw_face(t, world_to_clip);

}

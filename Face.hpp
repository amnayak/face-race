#pragma once

#include <string>
#include <array>
#include <map>
#include <vector>
#include "GL.hpp"
#include <glm/glm.hpp>
#include "Scene.hpp"

class Face {
  public:

	   Face(); //constructor
     ~Face(); //destructor

     //Attrib includes location within the vertex buffer of various attributes:
    // (exactly the parameters to glVertexAttribPointer)
     struct Attrib {
       GLint size = 0;
       GLenum type = 0;
       GLboolean normalized = GL_FALSE;
       GLsizei stride = 0;
       GLsizei offset = 0;

       Attrib() = default;
       Attrib(GLint size_, GLenum type_, GLboolean normalized_, GLsizei stride_, GLsizei offset_)
       : size(size_), type(type_), normalized(normalized_), stride(stride_), offset(offset_) { }
     };

     struct Vertex {
       glm::vec3 Position;
       glm::vec3 Normal;
       glm::u8vec4 Color;
       glm::vec2 TexCoord;
     };

    Attrib Position;
    Attrib Normal;
    Attrib Color;
    Attrib TexCoord;


     struct ShapeKey {
       GLuint start = 0; //TODO not really used
   		 GLuint count = 0;
       std::vector <glm::vec3> verts; //vertex data
     };

     size_t cur_vao_size;
     std::vector <Vertex> data_to_write; //stores linear combination of
                                            //weighted vertices
     std::vector <float> weights;
     std::vector <ShapeKey> key_frames; //stores all shapekeys

      GLuint vbo; //vertex buffer object
	    GLuint vao; //vertex array object

      void draw_face(Scene::Transform &t, glm::mat4 const &world_to_clip);
      void draw_face(Scene::Transform &t, Scene::Camera const *camera);

      GLuint total; //total num of vertices for index checks

      std::map< std::string, ShapeKey > frame_map; //TODO not really used

      void load_shape_key_data(); //loads shape key data in
      void recalculate_mesh_data(); //recalculates linear combo, and stores into
                                    //data_to_write
};

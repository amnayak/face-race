#pragma once

#include <string>
#include <array>
#include <map>
#include <vector>
#include "GL.hpp"
#include <glm/glm.hpp>
#include "Scene.hpp"
#include "MeshBuffer.hpp"

class ShapeKeyMesh {
public:
    ShapeKeyMesh(std::string filename, MeshBuffer *const mesh);
    ~ShapeKeyMesh();

    MeshBuffer *const mesh;

    struct ShapeKey {
        std::string name;
        uint32_t start_vertex;
        unsigned short index;

        ShapeKey(std::string name, uint32_t start_vertex, unsigned short index) : name(name), start_vertex(start_vertex), index(index) { }
    };

    // Buffer of raw vertex positions, for each shape key
    // Shape keys index into this array
    std::vector <glm::vec3> vertex_buf;
    std::vector <ShapeKey> key_frames;

    /* Map of shape keys by their name */
    std::map< std::string, ShapeKey > frame_map;

    /* recalculates linear combo, and stores into data_to_write */
    void recalculate_mesh_data(const std::vector <float> &weights);

    bool has_key_for_name(std::string const& name) {
        for(ShapeKey &cur : key_frames) {
            if(cur.name == name) {
                return true;
            }
        }

        return false;
    }

private:
    /* Generic data to write.  Packed with P/PN/PNC/PNCTVertex based
     * on mesh->draw_mode.
     */
    std::vector <char> data_to_write;
};

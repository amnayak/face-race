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
        unsigned short vgroup_mask;
        bool has_vgroup;

        ShapeKey() : name(""), start_vertex(0), index(0), vgroup_mask(0), has_vgroup(false) { }
        ShapeKey(std::string name, uint32_t start_vertex, unsigned short index, unsigned short vgroup_mask, bool has_vgroup) : name(name), start_vertex(start_vertex), index(index), vgroup_mask(vgroup_mask), has_vgroup(has_vgroup) { }
    };

    struct VertexGroup {
        std::string name;
        uint32_t start_index, end_index;
        unsigned short grp_index;

        VertexGroup(std::string name, uint32_t start_index, uint32_t end_index, unsigned short grp_index) : name(name), start_index(start_index), end_index(end_index), grp_index(grp_index) { }
    };

    // Buffer of raw vertex positions, for each shape key
    // Shape keys index into this array
    std::vector <glm::vec3> vertex_buf;
    std::vector <uint32_t> vgroups_buf;
    std::vector <ShapeKey> key_frames;
    std::vector <VertexGroup> vertex_groups;

    /* Map of shape keys by their name (NOT including reference) */
    std::map< std::string, ShapeKey > frame_map;
    std::map< std::string, VertexGroup > group_map;

    /* Reference shape key */
    ShapeKey reference_key;

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

    std::vector <float> norms;
};

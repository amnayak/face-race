#include "ShapeKeyMesh.hpp"
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

ShapeKeyMesh::ShapeKeyMesh(std::string filename, MeshBuffer *const mesh) : mesh(mesh) {
    if(mesh->meshes.size() != 1)
        throw std::runtime_error("meshbuffer must have exactly one mesh to use shape keys");

    std::ifstream file(data_path(filename), std::ios::binary);

    std::cout << "Reading keys from file " << filename << "..." << std::flush; 

    read_chunk(file, "keys", &vertex_buf);

    std::vector<char> strings_buf;
    struct IndexEntry {
        uint32_t name_begin, name_end;
        uint32_t vertex_begin, vertex_end;
    };
    std::vector<IndexEntry> idx_buf;

    read_chunk(file, "str0", &strings_buf);
    read_chunk(file, "idx0", &idx_buf);

    for (auto const &entry : idx_buf) {
        if (!(entry.name_begin <= entry.name_end && entry.name_end <= strings_buf.size()))
            throw std::runtime_error("index entry has out-of-range name begin/end");

        MeshBuffer::Mesh& cur_mesh = (*mesh->meshes.begin()).second;

        // TODO: Allow more than 2 shapekeys
        if (!(entry.vertex_begin <= entry.vertex_end && entry.vertex_end <= vertex_buf.size()))
            throw std::runtime_error("index entry has out-of-range vertex start/count");
        
        if(entry.vertex_end - entry.vertex_begin != cur_mesh.count)
            throw std::runtime_error("shape key entry does not match mesh buffer size");

        std::string name(&strings_buf[0] + entry.name_begin, &strings_buf[0] + entry.name_end);
        ShapeKey key(name, entry.vertex_begin, key_frames.size());

        key_frames.push_back(key);
        frame_map.insert({name, key});
    }

    std::cout << "Done." << std::endl;
}

ShapeKeyMesh::~ShapeKeyMesh() {
  //TODO
}

//recalculates the weighted linear combination of the vertices
// and uploads to GPU
void ShapeKeyMesh::recalculate_mesh_data (const std::vector <float> &weights) {
    if(weights.size() != key_frames.size())
        throw std::runtime_error("incorrect weights array length");

    size_t sizeof_vertex;
    size_t offsetof_pos;
    switch(mesh->format) {
        case P:
        sizeof_vertex = sizeof(PVertex);
        offsetof_pos = offsetof(PVertex, Position);
        break;
        case PN:
        sizeof_vertex = sizeof(PNVertex);
        offsetof_pos = offsetof(PNVertex, Position);
        break;
        case PNC:
        sizeof_vertex = sizeof(PNCVertex);
        offsetof_pos = offsetof(PNCVertex, Position);
        break;
        case PNCT:
        sizeof_vertex = sizeof(PNCTVertex);
        offsetof_pos = offsetof(PNCTVertex, Position);
        break;
    }

    size_t vcount = (*(mesh->meshes.begin())).second.count;
    data_to_write.resize(sizeof_vertex * vcount);

    std::copy(mesh->data.begin(), mesh->data.end(), data_to_write.begin());

    for (int v = 0; v < vcount; v++) {
        glm::vec3 *pos = (glm::vec3 *)((char *)data_to_write.data() + (v * sizeof_vertex) + offsetof_pos);

        float weight_sum = 0.0f;
        for(int i = 0; i < key_frames.size(); i++){
            *pos += vertex_buf[v] * weights[i];
            weight_sum += weights[i];
        }

        assert (weight_sum == 1.0f);
    }

    mesh->update_vertex_data(data_to_write);
}
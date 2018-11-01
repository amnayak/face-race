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
    struct SKIndexEntry {
        uint32_t name_begin, name_end;
        uint32_t vgroup;
        uint32_t vertex_begin, vertex_end;
    };
    struct IndexEntry {
        uint32_t name_begin, name_end;
        uint32_t vertex_begin, vertex_end;
    };
    std::vector<SKIndexEntry> idx_buf;
    std::vector<IndexEntry> gidx_buf;

    read_chunk(file, "str0", &strings_buf);
    read_chunk(file, "idx0", &idx_buf);
    read_chunk(file, "grp0", &vgroups_buf);
    read_chunk(file, "idx1", &gidx_buf);

    MeshBuffer::Mesh& cur_mesh = (*mesh->meshes.begin()).second;

    for (auto const &entry : idx_buf) {
        if (!(entry.name_begin <= entry.name_end && entry.name_end <= strings_buf.size()))
            throw std::runtime_error("index entry has out-of-range name begin/end");

        // TODO: Allow more than 2 shapekeys
        if (!(entry.vertex_begin <= entry.vertex_end && entry.vertex_end <= vertex_buf.size()))
            throw std::runtime_error("index entry has out-of-range vertex start/count");
        
        if(entry.vertex_end - entry.vertex_begin != cur_mesh.count)
            throw std::runtime_error("shape key entry does not match mesh buffer size");

        std::string name(&strings_buf[0] + entry.name_begin, &strings_buf[0] + entry.name_end);
        ShapeKey key(name, entry.vertex_begin, key_frames.size(), entry.vgroup - 1, entry.vgroup != 0);

        key_frames.push_back(key);
        frame_map.insert({name, key});
    }

    for (auto const &entry : gidx_buf) {
        if (!(entry.name_begin <= entry.name_end && entry.name_end <= strings_buf.size()))
            throw std::runtime_error("index entry has out-of-range name begin/end");

        if (!(0 <= entry.vertex_end && entry.vertex_end < cur_mesh.count))
            throw std::runtime_error("index entry has out-of-range vertex start/count");

        std::string name(&strings_buf[0] + entry.name_begin, &strings_buf[0] + entry.name_end);
        VertexGroup ngrp(name, entry.vertex_begin, entry.vertex_end, vertex_groups.size());

        vertex_groups.push_back(ngrp);
        group_map.insert({name, ngrp});
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
    assert(sizeof_vertex * vcount == mesh->data.size());

    if(data_to_write.size() != mesh->data.size())
        data_to_write = mesh->data; //copy

    for (int v = 0; v < vcount; v++) {
        glm::vec3 *pos = (glm::vec3 *)((char *)data_to_write.data() + (v * sizeof_vertex) + offsetof_pos);
        *pos = glm::vec3(0,0,0);
    }

    std::vector <float> norms;
    norms.resize(vcount);

    auto apply_key = [&](int i, int v) {
        glm::vec3 *pos = (glm::vec3 *)((char *)data_to_write.data() + (v * sizeof_vertex) + offsetof_pos);
        *pos += vertex_buf[v + i * vcount] * weights[i];

        norms[v] += weights[i];
    };
    for(int i = 0; i < key_frames.size(); i++) {
        if(key_frames[i].vgroup_mask >= vertex_groups.size()) {
            // There is no vertex group associated with this shape key.
            // So just apply this shape to all vertices
            for (int v = 0; v < vcount; v++)
                apply_key(i, v);
        } else {
            VertexGroup &vgroup = vertex_groups[key_frames[i].vgroup_mask];
            for (uint32_t cur = vgroup.start_index; cur <= vgroup.end_index; ++cur) {
                apply_key(i, vgroups_buf[cur]);
            }
        }   
    }
    // Normalize everything to (0, 1) range
    for (int v = 0; v < vcount; v++) {
        glm::vec3 *pos = (glm::vec3 *)((char *)data_to_write.data() + (v * sizeof_vertex) + offsetof_pos);
        if(norms[v] > 0.001f)
            *pos /= norms[v];
    }

    mesh->update_vertex_data(data_to_write);
}

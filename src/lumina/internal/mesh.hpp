#pragma once

#include <unordered_map>
#include <string>

#include "material.hpp"

namespace lumina {

struct mesh {
    std::vector<vec3f32> vertices;
    std::vector<vec2f32> texcoords;
    std::vector<vec3f32> normals;

    // required in .obj file
    std::vector<vec3u32> vertex_indices;
    // some polygons may have no texcoords or completely empty
    std::vector<std::optional<vec3u32>> texcoord_indices;
    // all polygons should have its normal but could be empty
    std::vector<std::optional<vec3u32>> normal_indices;

    // group name -> polygon count
    std::unordered_map<std::string, u32> mesh_groups;
    // group name -> material information
    std::unordered_map<std::string, material> mat_info;

    // precomputed dot products to calculate barycentric coordinates
    // (d00, d01, d11, denominator)
    std::vector<vec4f32> bary_dots;

    // forbid default construction
    mesh() = delete;

    explicit mesh(
        std::vector<vec3f32>&& vertices,
        std::vector<vec2f32>&& texcoords,
        std::vector<vec3f32>&& normals,
        std::vector<vec3u32>&& vertex_indices,
        std::vector<std::optional<vec3u32>>&& texcoord_indices,
        std::vector<std::optional<vec3u32>>&& normal_indices,
        std::unordered_map<std::string, u32> mesh_groups
    ) noexcept :
        vertices(vertices),
        texcoords(texcoords),
        normals(normals),
        vertex_indices(vertex_indices),
        texcoord_indices(texcoord_indices),
        normal_indices(normal_indices),
        mesh_groups(mesh_groups),
        mat_info(),
        bary_dots(vertex_indices.size())
    {
        for(size_t i = 0; i < vertex_indices.size(); ++i) {
            auto index = vertex_indices[i];
            auto v0 = vertices[index.y] - vertices[index.x];
            auto v1 = vertices[index.z] - vertices[index.x];
            auto d00 = dot(v0, v0);
            auto d01 = dot(v0, v1);
            auto d11 = dot(v1, v1);
            auto denominator = d00 * d11 - d01 * d01;

            bary_dots[i] = vec4f32(d00, d01, d11, denominator);
        }

        for(const auto& [name, range] : mesh_groups) {
            mat_info[name] = lumina::material{};
        }
    }

    // forbid copy
    mesh(const mesh&) = delete;
    mesh& operator=(const mesh&) = delete;
    mesh(mesh&&) = default;
    mesh& operator=(mesh&&) = default;

    bool add_material(const std::string& name, const material& material) {
        if(mesh_groups.contains(name)) {
            mat_info[name] = material;
            return true;
        }
        else {
            return false;
        }
    }

    void statistics() const {
        std::cout << std::format("# of vertices: {}, # of texcoords: {}, # of normals: {}, # of polygons: {}\n", vertices.size(), texcoords.size(), normals.size(), vertex_indices.size());
        for(const auto& [name, count] : mesh_groups) {
            std::cout << std::format("group name: {}, count {}\n", name, count);
        }
        for(const auto& [name, mat] : mat_info) {
            std::cout << std::format("group name: {}, material: {}\n", name, mat);
        }
        std::cout << std::flush;
    }

    vec2f32 texcoord(const vec3f32& p, u32 index_index) const {
        const auto& vertex_index = vertex_indices[index_index];
        auto v0 = vertices[vertex_index.y] - vertices[vertex_index.x];
        auto v1 = vertices[vertex_index.z] - vertices[vertex_index.x];

        auto v2 = p - vertices[vertex_index.x];
        auto d20 = dot(v2, v0);
        auto d21 = dot(v2, v1);

        auto v = (bary_dots[index_index].z * d20 - bary_dots[index_index].y * d21) / bary_dots[index_index].w;
        auto w = (bary_dots[index_index].x * d21 - bary_dots[index_index].y * d20) / bary_dots[index_index].w;
        auto u = 1.0f - v - w;

        // polygon has texcoords -> interpolate vertex texcoords
        if(texcoord_indices[index_index]) {
            const auto& texcoord_index = *texcoord_indices[index_index];
            return u * texcoords[texcoord_index.x] + v * texcoords[texcoord_index.y] + w * texcoords[texcoord_index.z];
        }
        // otherwise -> return (u, v) as texcoords (meaningless)
        else {
            return vec2f32(u, v);
        }
    }

    vec3f32 normal(const vec3f32& p, u32 index_index) const {
        const auto& vertex_index = vertex_indices[index_index];
        auto v0 = vertices[vertex_index.y] - vertices[vertex_index.x];
        auto v1 = vertices[vertex_index.z] - vertices[vertex_index.x];

        // polygon has normals -> calculate barycentric coordinate and interpolate vertex normals
        if(normal_indices[index_index]) {
            auto v2 = p - vertices[vertex_index.x];
            auto d20 = dot(v2, v0);
            auto d21 = dot(v2, v1);

            auto v = (bary_dots[index_index].z * d20 - bary_dots[index_index].y * d21) / bary_dots[index_index].w;
            auto w = (bary_dots[index_index].x * d21 - bary_dots[index_index].y * d20) / bary_dots[index_index].w;
            auto u = 1.0f - v - w;

            const auto& normal_index = *normal_indices[index_index];

            return normalize(u * normals[normal_index.x] + v * normals[normal_index.y] + w * normals[normal_index.z]);
        }
        // otherwise -> calculate normal from triangle
        else {
            return normalize(cross(v0, v1));
        }
    }

    material material(u32 index_index) const {
        for(const auto& [name, count] : mesh_groups) {
            if(index_index < count) {
                return mat_info.at(name);
            }
            else {
                index_index -= count;
            }
        }

        return lumina::material{};
    }
};

}
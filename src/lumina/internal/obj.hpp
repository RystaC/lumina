#pragma once

#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "base.hpp"
#include "vector.hpp"

namespace lumina {

inline std::string_view read_token(const std::string_view& str) {
    // pre-condition -> removed all spaces before current token)
    auto result = str.substr(0, str.find_first_of(' '));
    return result;
}

inline bool seek_token(std::string_view& str) {
    // no more token -> end of line
    if(str.find_first_of(' ') == std::string_view::npos) {
        return false;
    }

    // remove current token
    str.remove_prefix(str.find_first_of(' '));
    // remove spaces for next token
    str.remove_prefix(str.find_first_not_of(' '));

    return true;
}

inline std::tuple<u32, std::optional<u32>, std::optional<u32>> read_index(std::string_view& str, u32 vertex_count, u32 texcoord_count, u32 normal_count) {
    // v
    if(str.find_first_of('/') == std::string_view::npos) {
        s32 v{};
        std::from_chars(str.data(), str.data() + str.length(), v);
        v = v < 0 ? static_cast<s32>(vertex_count) + v : v - 1;

        return { v, std::nullopt, std::nullopt };
    }

    auto v_str = str.substr(0, str.find_first_of('/'));
    str.remove_prefix(str.find_first_of('/') + 1);

    // v/t
    if(str.find_first_of('/') == std::string_view::npos) {
        s32 v{};
        s32 t{};
        std::from_chars(v_str.data(), v_str.data() + v_str.length(), v);
        std::from_chars(str.data(), str.data() + str.length(), t);
        v = v < 0 ? static_cast<s32>(vertex_count) + v : v - 1;
        t = t < 0 ? static_cast<s32>(texcoord_count) + t : t - 1;

        return { v, t, std::nullopt };
    }

    auto t_str = str.substr(0, str.find_first_of('/'));
    str.remove_prefix(str.find_first_of('/') + 1);

    // v//n
    if(t_str.empty()) {
        s32 v{};
        s32 n{};
        std::from_chars(v_str.data(), v_str.data() + v_str.length(), v);
        std::from_chars(str.data(), str.data() + str.length(), n);
        v = v < 0 ? static_cast<s32>(vertex_count) + v : v - 1;
        n = n < 0 ? static_cast<s32>(normal_count) + n : n - 1;

        return { v, std::nullopt, n };
    }
    // v/t/n
    else {
        s32 v{};
        s32 t{};
        s32 n{};
        std::from_chars(v_str.data(), v_str.data() + v_str.length(), v);
        std::from_chars(t_str.data(), t_str.data() + t_str.length(), t);
        std::from_chars(str.data(), str.data() + str.length(), n);
        v = v < 0 ? static_cast<s32>(vertex_count) + v : v - 1;
        t = t < 0 ? static_cast<s32>(texcoord_count) + t : t - 1;
        n = n < 0 ? static_cast<s32>(normal_count) + n : n - 1;

        return { v, t, n };
    }
}

// (vertices, indices, group mapping (group name, start index, end index + 1))
inline 
std::tuple<
    std::vector<vec3f32>,
    std::vector<vec2f32>,
    std::vector<vec3f32>,
    std::vector<vec3u32>,
    std::vector<std::optional<vec3u32>>,
    std::vector<std::optional<vec3u32>>,
    std::unordered_map<std::string, u32>
> load_obj(const char* path) {
    std::FILE* fp = std::fopen(path, "r");
    if(!fp) {
        std::clog << std::format("could not read file: {}", path) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<vec3f32> vertices{};
    std::vector<vec2f32> texcoords{};
    std::vector<vec3f32> normals{};
    std::vector<vec3u32> vertex_indices{};
    std::vector<std::optional<vec3u32>> texcoord_indices{};
    std::vector<std::optional<vec3u32>> normal_indices{};

    // group name -> polygon count
    std::unordered_map<std::string, u32> mesh_groups{};

    // maximum line length of .obj file
    constexpr u32 BUF_SIZE = 256;
    std::array<char, BUF_SIZE> buf{};
    u32 line{};

    std::string current_group{};
    u32 current_count{};

    while(!std::feof(fp)) {
        buf.fill('\0');

        std::fgets(buf.data(), static_cast<int>(buf.size()), fp);
        ++line;

        if(buf[BUF_SIZE-2] != '\0') {
            std::clog << std::format("buffer overflow. parser expects up to {} characters for each line.", BUF_SIZE-1) << std::endl;
            std::exit(EXIT_FAILURE);
        }

        std::string_view str(buf.data());

        // line if not EOF -> remove line feed
        if(str.find('\n') != std::string_view::npos) {
            str.remove_suffix(1);
        }

        // remove spaces at the end
        auto last = str.find_last_not_of(' ');
        if(last != std::string_view::npos && last + 1 < str.length()) {
            last += 1;
            str.remove_suffix(str.length() - last);
        }

        // empty line -> skip
        if(str.empty() || str.find_first_not_of(' ') == std::string_view::npos) {
            continue;
        }

        // remove front spaces
        str.remove_prefix(str.find_first_not_of(' '));

        // comment line -> skip
        if(str[0] == '#') {
            continue;
        }

        auto head = read_token(str);

        // vertex
        if(head == "v") {
            seek_token(str);

            auto x_str = read_token(str);
            seek_token(str);
            auto y_str = read_token(str);
            seek_token(str);
            auto z_str = read_token(str);
            f32 x{};
            f32 y{};
            f32 z{};
            std::from_chars(x_str.data(), x_str.data() + x_str.length(), x);
            std::from_chars(y_str.data(), y_str.data() + y_str.length(), y);
            std::from_chars(z_str.data(), z_str.data() + z_str.length(), z);

            vertices.push_back({x, y, z});
        }
        // texcoord
        else if(head == "vt") {
            seek_token(str);

            auto u_str = read_token(str);
            seek_token(str);
            auto v_str = read_token(str);
            f32 u{};
            f32 v{};
            std::from_chars(u_str.data(), u_str.data() + u_str.length(), u);
            std::from_chars(v_str.data(), v_str.data() + v_str.length(), v);

            texcoords.push_back({u, v});
        }
        // normal
        else if(head == "vn") {
            seek_token(str);

            auto x_str = read_token(str);
            seek_token(str);
            auto y_str = read_token(str);
            seek_token(str);
            auto z_str = read_token(str);
            f32 x{};
            f32 y{};
            f32 z{};
            std::from_chars(x_str.data(), x_str.data() + x_str.length(), x);
            std::from_chars(y_str.data(), y_str.data() + y_str.length(), y);
            std::from_chars(z_str.data(), z_str.data() + z_str.length(), z);

            normals.push_back({x, y, z});
        }
        // face (index)
        else if(head == "f") {
            seek_token(str);

            auto i0_str = read_token(str);
            seek_token(str);
            auto i1_str = read_token(str);
            seek_token(str);
            auto i2_str = read_token(str);
            
            auto [v0, t0, n0] = read_index(i0_str, vertices.size(), texcoords.size(), normals.size());
            auto [v1, t1, n1] = read_index(i1_str, vertices.size(), texcoords.size(), normals.size());
            auto [v2, t2, n2] = read_index(i2_str, vertices.size(), texcoords.size(), normals.size());

            vertex_indices.push_back({v0, v1, v2});
            if(t0 && t1 && t2) {
                texcoord_indices.push_back(vec3u32(*t0, *t1, *t2));
            }
            else {
                texcoord_indices.push_back(std::nullopt);
            }
            if(n0 && n1 && n2) {
                normal_indices.push_back(vec3u32(*n0, *n1, *n2));
            }
            else {
                normal_indices.push_back(std::nullopt);
            }

            // face has 4 indices
            if(seek_token(str)) {
                auto i3_str = read_token(str);
                auto [v3, t3, n3] = read_index(i3_str, vertices.size(), texcoords.size(), normals.size());

                vertex_indices.push_back({v3, v0, v2});
                if(t3 && t0 && t2) {
                    texcoord_indices.push_back(vec3u32(*t3, *t0, *t2));
                }
                else {
                    texcoord_indices.push_back(std::nullopt);
                }
                if(n3 && n0 && n2) {
                    normal_indices.push_back(vec3u32(*n3, *n0, *n2));
                }
                else {
                    normal_indices.push_back(std::nullopt);
                }
            }
        }
        // group
        else if(head == "g") {
            seek_token(str);

            auto group_name = read_token(str);
            seek_token(str);

            if(current_group.empty()) {
                current_group = group_name;
                current_count = 0;
            }
            else {
                mesh_groups[current_group] = static_cast<u32>(vertex_indices.size()) - current_count;

                current_group = group_name;
                current_count = static_cast<u32>(vertex_indices.size());
            }
        }
        // other token -> skip
        else {
            continue;
        }
    }

    // .obj file has no groups -> register group with all polygons
    if(mesh_groups.empty()) {
        mesh_groups[""] = static_cast<u32>(vertex_indices.size());
    }
    else {
        mesh_groups[current_group] = static_cast<u32>(vertex_indices.size()) - current_count;
    }

    std::fclose(fp);

    return { vertices, texcoords, normals, vertex_indices, texcoord_indices, normal_indices, mesh_groups };
}

}
#pragma once

#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string_view>
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

// only read vertex index (others are not needed currently)
inline u32 read_index(std::string_view& str, u32 vertex_count) {
    auto v_str = str.substr(0, str.find_first_of('/'));

    u32 v{};
    std::from_chars(v_str.data(), v_str.data() + v_str.length(), v);
    v = v < 0 ? vertex_count + v : v - 1;

    return v;
}

inline std::pair<std::vector<vec3f32>, std::vector<vec3u32>> load_obj(const char* path) {
    std::FILE* fp = std::fopen(path, "r");
    if(!fp) {
        std::clog << std::format("could not read file: {}", path) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<vec3f32> vertices{};
    std::vector<vec3u32> indices{};

    // maximum line length of .obj file
    constexpr u32 BUF_SIZE = 256;
    std::array<char, BUF_SIZE> buf{};
    u32 line{};

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
        // face (index)
        else if(head == "f") {
            seek_token(str);

            auto i0_str = read_token(str);
            seek_token(str);
            auto i1_str = read_token(str);
            seek_token(str);
            auto i2_str = read_token(str);
            
            auto i0 = read_index(i0_str, vertices.size());
            auto i1 = read_index(i1_str, vertices.size());
            auto i2 = read_index(i2_str, vertices.size());

            indices.push_back({i0, i1, i2});

            // face has 4 indices
            if(seek_token(str)) {
                auto i3_str = read_token(str);
                auto i3 = read_index(i3_str, vertices.size());

                indices.push_back({i3, i0, i2});
            }
        }
        // other token -> skip
        else {
            continue;
        }
    }

    std::fclose(fp);

    return { vertices, indices };
}

}
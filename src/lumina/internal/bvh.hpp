#pragma once

#include <algorithm>
#include <numeric>
#include <queue>
#include <vector>

#include "aabb.hpp"
#include "ray.hpp"
#include "intersect.hpp"

namespace lumina {

struct bvh_node {
    aabb left_box;
    aabb right_box;
    // 0 or negative -> index of primitive (leaf)
    // positive -> index of child nodes
    s32  left_index;
    s32  right_index;
};

inline std::ostream& operator<<(std::ostream& os, const bvh_node& bn) {
    os << std::format("left box: {}, right box: {}, left index: {}, right index: {}", bn.left_box, bn.right_box, bn.left_index, bn.right_index);
    return os;
}

class bvh {
    std::vector<bvh_node> nodes_;

public:
    bvh(const std::vector<vec3f32>& vertices, const std::vector<vec3u32>& indices);

    std::optional<std::pair<u32, f32>> trace(const std::vector<vec3f32>& vertices, const std::vector<vec3u32>& indices, const ray& r, f32 t_max) const;
};

}

template<>
struct std::formatter<lumina::bvh_node> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::bvh_node& bn, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "left box: {}, right box: {}, left index: {}, right index: {}", bn.left_box, bn.right_box, bn.left_index, bn.right_index);
    }
};
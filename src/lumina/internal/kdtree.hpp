#pragma once

#include <algorithm>
#include <stack>
#include <vector>

#include "vector.hpp"
#include "sphere.hpp"

namespace lumina {

class kdtree {
    static constexpr vec3f32 SENTRY_ = vec3f32(F32_MAX);

    std::vector<vec3f32> points_;
    u32 height_;

public:
    kdtree(std::vector<vec3f32> src);

    vec3f32 nn_search(const vec3f32& query) const;
};

}
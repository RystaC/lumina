#pragma once

#include "vector.hpp"
#include "ray.hpp"

namespace lumina {

struct sphere {
    vec3f32 center;
    f32 radius;

    constexpr sphere() noexcept : center(), radius() {}
    constexpr sphere(const vec3f32& center, float radius) noexcept : center(center), radius(radius) {}

    constexpr vec3f32 normal(const ray& r, f32 t) const noexcept {
        auto n = normalize(r[t] - center);
        if(dot(r.direction, n) > 0.0f) {
            return -n;
        }
        else {
            return n;
        }
    }
};

inline std::ostream& operator<<(std::ostream& os, const sphere& s) {
    os << std::format("center: {}, radius: {:.2f}", s.center, s.radius);
    return os;
}

}

template<>
struct std::formatter<lumina::sphere> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::sphere& s, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "center: {}, radius: {:.2f}", s.center, s.radius);
    }
};
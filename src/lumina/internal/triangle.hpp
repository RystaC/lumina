#pragma once

#include "vector.hpp"
#include "ray.hpp"

namespace lumina {

// front-face -> counter-clockwise
struct triangle {
    vec3f32 p0;
    vec3f32 p1;
    vec3f32 p2;

    constexpr triangle() noexcept : p0(), p1(), p2() {}
    constexpr triangle(const vec3f32& p0, const vec3f32& p1, const vec3f32& p2) noexcept : p0(p0), p1(p1), p2(p2) {}

    constexpr vec3f32 centroid() const noexcept {
        return (p0 + p1 + p2) / 3.0f;
    }

    constexpr vec3f32 normal(const ray& r, f32 t) const noexcept {
        auto n = normalize(cross(p1 - p0, p2 - p0));
        if(dot(r.direction, n) > 0.0f) {
            return -n;
        }
        else {
            return n;
        }
    }

    constexpr vec3f32 barycentric(const vec3f32 p) const noexcept {
        auto v0 = p1 - p0;
        auto v1 = p2 - p0;
        auto v2 = p - p0;
        auto d00 = dot(v0, v0);
        auto d01 = dot(v0, v1);
        auto d11 = dot(v1, v1);
        auto d20 = dot(v2, v0);
        auto d21 = dot(v2, v1);

        auto denominator = d00 * d11 - d01 * d01;

        auto v = (d11 * d20 - d01 * d21) / denominator;
        auto w = (d00 * d21 - d01 * d20) / denominator;
        auto u = 1.0f - v - w;

        return { u, v, w };
    }
};

inline std::ostream& operator<<(std::ostream& os, const triangle& t) {
    os << std::format("p0: {}, p1: {}, p2: {}", t.p0, t.p1, t.p2);
    return os;
}

}

template<>
struct std::formatter<lumina::triangle> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::triangle& t, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "p0: {}, p1: {}, p2: {}", t.p0, t.p1, t.p2);
    }
};
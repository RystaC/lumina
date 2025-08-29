#pragma once

#include <optional>

#include "aabb.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "triangle.hpp"

namespace lumina {

// ray-sphere
inline constexpr std::optional<f32> intersect(const ray& r, const sphere& s) noexcept {
    auto oc = s.center - r.origin;
    auto a = dot(r.direction, r.direction);
    auto b = dot(r.direction, oc);
    auto c = dot(oc, oc) - s.radius * s.radius;
    auto d = b * b - a * c;
    if(d < 0) {
        return std::nullopt;
    }
    else {
        return (b - std::sqrt(d)) / a;
    }
}

inline constexpr std::optional<f32> intersect(const sphere& s, const ray& r) noexcept {
    return intersect(r, s);
}

// ray-aabb
inline constexpr std::optional<f32> intersect(const ray& r, const aabb& b) noexcept {
    f32 t_min = F32_MIN;
    f32 t_max = F32_MAX;
    for(auto i = 0; i < 3; ++i) {
        auto odd = 1.0f / r.direction[i];
        auto t0 = (b.min[i] - r.origin[i]) * odd;
        auto t1 = (b.max[i] - r.origin[i]) * odd;
        if(t0 > t1) {
            std::swap(t0, t1);
        }

        if(t0 > t_min) {
            t_min = t0;
        }
        if(t1 < t_max) {
            t_max = t1;
        }

        if(t_min > t_max) {
            return std::nullopt;
        }
    }

    return t_min;
}

inline constexpr std::optional<f32> intersect(const aabb& b, const ray& r) noexcept {
    return intersect(r, b);
}

// ray-triangle
inline constexpr std::optional<f32> intersect(const ray& r, const triangle& t) noexcept {
    auto e1 = t.p1 - t.p0;
    auto e2 = t.p2 - t.p0;

    auto alpha = cross(r.direction, e2);
    auto det = dot(e1, alpha);

    if(-F32_MACHINE_EPS < det && det < F32_MACHINE_EPS) {
        return std::nullopt;
    }

    auto inv_det = 1.0f / det;
    auto _r = r.origin - t.p0;
    auto u = dot(alpha, _r) * inv_det;
    if(u < 0.0f || 1.0f < u) {
        return std::nullopt;
    }

    auto beta = cross(_r, e1);
    auto v = dot(r.direction, beta) * inv_det;
    if(v < 0.0f || 1.0f < u + v) {
        return std::nullopt;
    }

    auto _t = dot(e2, beta) * inv_det;
    if(_t < 0.0f) {
        return std::nullopt;
    }

    return _t;
}

inline constexpr std::optional<f32> intersect(const triangle& t, const ray& r) noexcept {
    return intersect(r, t);
}

}
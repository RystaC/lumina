#pragma once

#include <random>

#include "vector.hpp"
#include "ray.hpp"

namespace lumina {

class camera {
    vec3f32 du_;
    vec3f32 dv_;
    vec3f32 first_pixel_;

public:
    vec3f32 from;
    vec3f32 at;
    vec3f32 up;

    camera(
        const vec3f32& from,
        const vec3f32& at,
        const vec3f32& up,
        f32 fov,
        u32 width,
        u32 height
    )
    : from(from), at(at), up(up)
    {
        auto focal_length = (from - at).norm();
        auto h = std::tan(radian(fov) / 2.0f);
        auto vp_height = 2.0f * h * focal_length;
        auto vp_width = vp_height * (f32(width) / f32(height));

        auto w = normalize(from - at);
        auto u = normalize(cross(up, w));
        auto v = cross(w, u);

        auto vp_u = u * vp_width;
        auto vp_v = -v * vp_height;

        du_ = vp_u / f32(width);
        dv_ = vp_v / f32(height);

        auto vp_upper_left = from - (w * focal_length) - vp_u / 2.0f - vp_v / 2.0f;
        first_pixel_ = vp_upper_left + 0.5f * (du_ + dv_);
    }

    ray generate_ray(u32 i, u32 j) {
        auto origin = first_pixel_ + (f32(i) * du_) + (f32(j) * dv_);
        auto direction = normalize(origin - from);

        return {origin, direction};
    }

    // for multi-sampling
    template<class RandGen>
    ray generate_ray(u32 i, u32 j, RandGen& rng) {
        f32 offset_x{};
        f32 offset_y{};
        std::uniform_real_distribution<f32> offset(-0.5f, 0.5f);
        offset_x = offset(rng);
        offset_y = offset(rng);

        auto origin = first_pixel_ + ((f32(i) + offset_x) * du_) + ((f32(j) + offset_y) * dv_);
        auto direction = normalize(origin - from);

        return {origin, direction};
    }
};

inline std::ostream& operator<<(std::ostream& os, const camera& c) {
    os << std::format("from: {}, at: {}, up: {}", c.from, c.at, c.up);
    return os;
}

}

template<>
struct std::formatter<lumina::camera> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::camera& c, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "from: {}, at: {}, up: {}", c.from, c.at, c.up);
    }
};
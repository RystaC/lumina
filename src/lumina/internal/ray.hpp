#pragma once

#include "vector.hpp"

namespace lumina {

struct ray {
    vec3f32 origin;
    vec3f32 direction;

    constexpr ray() noexcept : origin(), direction() {}
    constexpr ray(const vec3f32& origin, const vec3f32& direction) noexcept : origin(origin), direction(direction) {}

    constexpr vec3f32 operator[](f32 t) const noexcept {
        return origin + t * direction;
    }
};

inline std::ostream& operator<<(std::ostream& os, const ray& r) {
    os << std::format("origin: {}, direction: {}", r.origin, r.direction);
    return os;
}

}

template<>
struct std::formatter<lumina::ray> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::ray& r, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "origin: {}, direction: {}", r.origin, r.direction);
    }
};
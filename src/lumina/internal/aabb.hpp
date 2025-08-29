#pragma once

#include "vector.hpp"
#include "triangle.hpp"

namespace lumina {

struct aabb {
    vec3f32 min;
    vec3f32 max;

    // initial value -> invalid box
    constexpr aabb() noexcept : min(F32_MAX), max(F32_MIN) {}
    constexpr aabb(const vec3f32& min, const vec3f32& max) noexcept : min(min), max(max) {}
    constexpr aabb(const triangle& t) noexcept : min(lumina::min(lumina::min(t.p0, t.p1), t.p2)), max(lumina::max(lumina::max(t.p0, t.p1), t.p2)) {}

    // empty box (min == max) is allowed
    constexpr bool is_valid() const noexcept {
        return ((min.x <= max.x) && (min.y <= max.y) && (min.z <= max.z));
    }

    constexpr operator bool() const noexcept { return is_valid(); }

    constexpr vec3f32 centroid() const noexcept {
        return (min + max) / 2.0f;
    }

    constexpr float area() const noexcept {
        auto d = max - min;
        return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
    }

    // merge operation
    constexpr aabb& operator+=(const aabb& b) {
        min = lumina::min(min, b.min);
        max = lumina::max(max, b.max);

        return *this;
    }
};

inline constexpr aabb operator+(const aabb& a, const aabb& b) {
    return aabb(a) += b;
}

inline std::ostream& operator<<(std::ostream& os, const aabb& b) {
    os << std::format("min: {}, max: {}", b.min, b.max);
    return os;
}

}

template<>
struct std::formatter<lumina::aabb> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::aabb& b, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "min: {}, max: {}", b.min, b.max);
    }
};
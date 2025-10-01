#pragma once

#include <random>

#include "vector.hpp"
#include "sampling.hpp"

namespace lumina {

struct material {
    vec3f32 albedo;
    vec3f32 emission;
    f32 roughness;
    f32 refractive_index;
};

inline std::ostream& operator<<(std::ostream& os, const material& m) {
    os << std::format("albedo: {}, emission: {}, roughness: {}, refractive index: {}", m.albedo, m.emission, m.roughness, m.refractive_index);
    return os;
}

}

template<>
struct std::formatter<lumina::material> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::material& m , std::format_context& ctx) const {
        return std::format_to(ctx.out(), "albedo: {}, emission: {}, roughness: {}, refractive index: {}", m.albedo, m.emission, m.roughness, m.refractive_index);
    }
};
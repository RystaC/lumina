#pragma once

#include "triangle.hpp"

namespace lumina {

// orthonormal basis
constexpr inline vec3f32 onb(const vec3f32& n, const vec3f32& v) {
    vec3f32 axis = std::abs(n.x) > 0.001f ? vec3f32(0.0f, 1.0f, 0.0f) : vec3f32(1.0f, 0.0f, 0.0f);
    auto t = normalize(cross(axis, n));
    auto s = cross(n, t);

    return normalize(s * v.x + t * v.y + n * v.z);
}

// spherical coordinate system
// x = sin_theta * cos_phi
// y = sin_theta * sin_phi
// z = cos_theta

// reference: https://rayspace.xyz/CG/contents/geometry_sampling_implementation/

// vector n should be normalized
template<class RandGen>
inline vec3f32 sample_uniform_sphere(const vec3f32& n, RandGen& rng) {
    std::uniform_real_distribution<f32> r{};

    auto u1 = r(rng);
    auto u2 = r(rng);

    auto cos_theta = 1.0f - 2.0f * u1;
    auto phi = 2.0f * F32_PI * u2;

    auto sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);


    auto x = sin_theta * std::cos(phi);
    auto y = sin_theta * std::sin(phi);
    auto z = cos_theta;

    return onb(n, {x, y, z});
}

constexpr inline f32 sample_uniform_sphere_pdf(const vec3f32&) {
    return 1.0f / (4.0f * F32_PI);
}

// vector n should be normalized
template<class RandGen>
inline vec3f32 sample_uniform_hemisphere(const vec3f32& n, RandGen& rng) {
    std::uniform_real_distribution<f32> r{};

    auto u1 = r(rng);
    auto u2 = r(rng);

    auto cos_theta = u1;
    auto phi = 2.0f * F32_PI * u2;

    auto sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);

    auto x = sin_theta * std::cos(phi);
    auto y = sin_theta * std::sin(phi);
    auto z = cos_theta;

    return onb(n, {x, y, z});
}

constexpr inline f32 sample_uniform_hemisphere_pdf(const vec3f32&) {
    return 1.0f / (2.0f * F32_PI);
}

// vector n should be normalized
template<class RandGen>
inline vec3f32 sample_cosine_hemisphere(const vec3f32& n, RandGen& rng) {
    std::uniform_real_distribution<f32> r{};

    auto u1 = r(rng);
    auto u2 = r(rng);

    auto cos_theta = std::sqrt(u1);
    auto phi = 2.0f * F32_PI * u2;

    auto sin_theta = std::sqrt(1.0f - u1);

    auto x = cos_theta * std::cos(phi);
    auto y = cos_theta * std::sin(phi);
    auto z = sin_theta;

    return onb(n, {x, y, z});
}

template<class RandGen>
inline vec3f32 sample_uniform_rectangle(const vec3f32& o, const vec3f32& a, const vec3f32& b, RandGen& rng) {
    std::uniform_real_distribution<f32> r{};

    auto u1 = r(rng);
    auto u2 = r(rng);

    return normalize(o + u1 * a + u2 * b);
}

constexpr inline f32 sample_uniform_rectangle_pdf(const vec3f32& a, const vec3f32& b) {
    return 1.0f / norm(cross(a, b));
}

// o = p0, a = p1 - p0, b = p2 - p0
template<class RandGen>
inline vec3f32 sample_uniform_triangle(const vec3f32& o, const vec3f32& a, const vec3f32& b, RandGen& rng) {
    std::uniform_real_distribution<f32> r{};

    auto u1 = r(rng);
    auto u2 = r(rng);

    auto t_a = 1.0f - std::sqrt(u1);
    auto t_b = (1.0f - t_a) * u2;

    return normalize(o + t_a * a + t_b * b);
}

constexpr inline f32 sample_uniform_triangle_pdf(const vec3f32& a, const vec3f32& b) {
    return 2.0f / norm(cross(a, b));
}

// Eric Heitz - "A Low-Distortion Map Between Triangle and Square", 2019
template<class RandGen>
inline vec3f32 sample_heitz_triangle(const vec3f32& p0, const vec3f32& p1, const vec3f32& p2, RandGen& rng) {
    std::uniform_real_distribution<f32> r{};

    auto u1 = r(rng);
    auto u2 = r(rng);

    auto t0 = 0.5f * u1;
    auto t1 = 0.5f * u2;
    auto t_off = t1 - t0;

    if(t_off > 0.0f) {
        t1 -= t_off;
    }
    else {
        t0 -= t_off; 
    }

    return normalize(t0 * p0 + t1 * p1 + (1.0f - t0 - t1) * p2);
}

}
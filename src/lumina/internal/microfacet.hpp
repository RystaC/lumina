#pragma once

#include "sampling.hpp"

namespace lumina {

// ray sampling functions

// microfacet model (isotrophic)
// from: Bruce Walter, Stephen R. Marschner, Hongsong Li, Kenneth E. Torrance, - "Microfacet Models for Refraction through Rough Surfaces", 2007

// reflectance function F
// i: incoming ray (surface -> light), m: microsurface normal, eta_i: index of refraction for outside of meterial, eta_o: index of refraction for inside of meterial
inline f32 f(const vec3f32& i, const vec3f32& m, f32 eta_i, f32 eta_o) {
    auto c = std::abs(dot(i, m));

    auto d = (eta_o * eta_o) / (eta_i * eta_i) - 1.0f + (c * c);
    if(d < 0) {
        return 1.0f;
    }

    auto g = std::sqrt(d);

    auto numerator1 = g - c;
    auto denominator1 = g + c;
    auto numerator2 = c * (g + c) - 1.0f;
    auto denominator2 = c * (g - c) + 1.0f;

    return 0.5f * ((numerator1 * numerator1) / (denominator1 * denominator1)) * (1.0f + (numerator2 * numerator2) / (denominator2 * denominator2));
}

// shadowing-masking function G
inline f32 g1(const vec3f32& v, const vec3f32& m, const vec3f32& n, f32 alpha) {
    auto theta_v = std::acos(dot(v, n));

    auto coef = std::max(0.0f, dot(v, m) / dot(v, n));
    auto denominator = 1.0f + std::sqrt(1.0f + alpha * alpha * std::tan(theta_v) * std::tan(theta_v));

    return coef * (2.0f / denominator);
}

inline f32 g(const vec3f32& i, const vec3f32& o, const vec3f32& m, const vec3f32& n, f32 alpha) {
    return g1(i, m, n, alpha) * g1(o, m, n, alpha);
}

// microfacet distribution function D
// m: microsurface normal, n: macrosurface normal, alpha: surface parameter (usually roughness^2)
inline f32 d(const vec3f32& m, const vec3f32& n, f32 alpha) {
    auto theta_m = std::acos(dot(m, n));

    auto numerator = alpha * alpha * std::max(0.0f, dot(m, n));
    auto denominator1 = F32_PI * std::pow(std::cos(theta_m), 4.0f);
    auto denominator2 = (alpha * alpha + std::tan(theta_m) * std::tan(theta_m));

    return numerator / (denominator1 * (denominator2 * denominator2));
} 

// F(i, h)
// G(i, o, h)
// D(h)

// i: incoming ray (surface -> light)
// o: outgoing ray (surface -> eye)

// inline f32 brdf_mf(const vec3f32& i, const vec3f32& o, const vec3f32& n) {
//     auto h = normalize(i + o);

//     auto numerator = f(i, h) * g(i, o, h) * d(h);
//     auto denominator = 4.0f * std::abs(dot(i, n)) * std::abs(dot(o, n));

//     return numerator / denominator;
// }

// inline f32 btdf_mf(const vec3f32& i, const vec3f32& o, const vec3f32& n) {
//     auto h = normalize(i + o);
//     f32 eta_i, eta_o;

//     auto coef = (std::abs(dot(i, h)) * std::abs(dot(o, h))) / (std::abs(dot(i, n)) * std::abs(dot(o, n)));
//     auto numerator = eta_o * eta_o * (1.0f - f(i, h)) * g(i, o, h) * d(h);
//     auto denominator = eta_i * dot(i, h) + eta_o * dot(o, h);

//     return coef * (numerator / (denominator * denominator));
// }

// inline f32 bsdf_mf(const vec3f32& i, const vec3f32& o, const vec3f32& m) {
//     return brdf_mf(i, o, m) + btdf_mf(i, o, m);
// }

template<class RandGen>
inline std::tuple<vec3f32, vec3f32, f32> sample_ggx(const vec3f32& omega_o, const vec3f32& n, f32 roughness, RandGen& rng) {
    std::uniform_real_distribution<f32> u(0.0f, 1.0f);

    auto alpha = roughness * roughness;

    auto u1 = u(rng);
    auto u2 = u(rng);

    auto theta = std::atan(alpha * std::sqrt(u1) / std::sqrt(1.0f - u1));
    auto phi = 2.0f * F32_PI * u2;

    auto x = std::sin(theta) * std::cos(phi);
    auto y = std::sin(theta) * std::sin(phi);
    auto z = std::cos(theta);

    auto m = onb(n, {x, y, z});
    auto omega_i = reflect(-omega_o, m);

    auto pdf_val = (d(m, n, alpha) * std::abs(dot(m, n))) / (4.0f * std::abs(dot(omega_o, m)));

    return { m, omega_i, pdf_val };
}

}
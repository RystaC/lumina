#pragma once

#include <random>

#include "vector.hpp"

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

namespace lumina {

// from google gemini
// provisionally implementation

// specular reflectance
// n1: incomming refrective index, n2: outgoing refrective index
// n2 is 0 (and n1 is not 0) means material cannot refract
f32 f0_(f32 n1, f32 n2) {
    auto f = (n1 - n2) / (n1 + n2);
    return f * f;
}

// schlick's approximation
// f0: fresnel's reflection coefficient, cos_theta: dot product between l and h
// if f0 equals 1, material will always reflect
f32 fresnel_(f32 f0, f32 cos_theta) {
    return f0 + (1.0f - f0) * std::pow(1.0f - cos_theta, 5.0f);
}

// normal distribution function GGX (generalized Trowbridge-Reitz) model
f32 ndf_ggx_(const vec3f32& n, const vec3f32& m, f32 a) {
    auto a2 = a * a;
    auto nm = dot(n, m);
    auto denominator = nm * nm * (a2 - 1.0f) + 1.0f;
    return a2 / (F32_PI * denominator * denominator);
}

// GAF (geometry attenuation factor)

// schick-GGX model
f32 g1_schlick_ggx_(const vec3f32& nu, const vec3f32& n, f32 alpha) {
    auto nun = std::abs(dot(nu, n));
    auto a2 = alpha * alpha;
    auto denominator = nun + std::sqrt(a2 + nun * nun * (1.0f - a2));
    return 2.0f * nun / denominator;
}

// smith's model
f32 g1_smith_(const vec3f32& nu, const vec3f32& n, f32 alpha) {
    auto k = alpha * 0.5f;
    auto nun = dot(nu, n);
    return nun / (nun * (1.0f - k) + k);
}

f32 gaf_smith_(const vec3f32& l, const vec3f32& v, const vec3f32& n, f32 alpha) {
    return g1_smith_(l, n, alpha) * g1_smith_(v, n, alpha);
}

// cook-torrance model
// interface>
// l: vector to light, v: vector to eye, n: normal vector, m: microfacet normal vector, albedo: color of diffuse component
// roughness: roughness of specular reflection, n1: incoming reflective index, n2: outgoing reflective index

// bidirectional reflectance distribution function
vec3f32 brdf(const vec3f32& l, const vec3f32& v, const vec3f32& n, const vec3f32& m, const vec3f32& albedo, f32 alpha, f32 n1, f32 n2) {
    // fresnel's term
    auto f0 = f0_(n1, n2);
    auto f = fresnel_(f0, dot(l, m));

    // normal distribution function
    // GGX (generalized Trowbridge-Reitz) distribution
    auto d = ndf_ggx_(n, m, alpha);

    // geometry attenuation term
    auto g = gaf_smith_(l, v, n, alpha);

    auto ln = std::abs(dot(l, n));
    auto vn = std::abs(dot(v, n));

    // specular term
    auto specular = vec3f32(f * g * d / (4.0f * ln * vn));

    // diffuse term
    auto diffuse = albedo / F32_PI;

    return specular + diffuse;
}

// bidirectional transmittance distribution function
vec3f32 btdf(const vec3f32& l, const vec3f32& v, const vec3f32& n, const vec3f32& m, const vec3f32& albedo, f32 alpha, f32 n1, f32 n2) {
    // fresnel's term
    auto f0 = f0_(n1, n2);
    auto f = fresnel_(f0, dot(l, m));

    // NDF term
    auto d = ndf_ggx_(n, m, alpha);

    // geometry attenuation term
    auto g = gaf_smith_(l, v, n, alpha);

    auto eta_l = n2 / n1;
    auto eta_v = n1 / n2;

    auto ln = std::abs(dot(l, n));
    auto lm = dot(l, m);
    auto vm = dot(v, m);

    auto numerator = d * g * std::abs(lm * vm) * f;
    auto denominator = (eta_l * eta_l * lm + eta_v * vm) * ln;
    
    return vec3f32(numerator / denominator);
}

// probability density function
// interface>
// l: vector to light, v: vector to eye, n: macro normal, m: mircofacet normal, alpha: alpha value of material

// BRDF
f32 pdf_brdf(const vec3f32& l, const vec3f32& v, const vec3f32& n, const vec3f32& m, f32 alpha, f32 n1, f32 n2) {
    auto nm = std::abs(dot(n, m));
    auto lm = std::abs(dot(v, m));

    auto d = ndf_ggx_(n, m, alpha);

    return (d * nm) / (4.0f * lm);
}

// BTDF
f32 pdf_btdf(const vec3f32& l, const vec3f32& v, const vec3f32& n, const vec3f32& m, f32 alpha, f32 n1, f32 n2) {
    auto d = ndf_ggx_(n, m, alpha);

    auto eta_l = n2 / n1;
    auto eta_v = n1 / n2;

    auto lm = dot(l, m);
    auto vm = dot(v, m);

    auto denominator = (eta_l * eta_l * lm + eta_v * vm);
    return d * std::abs(lm * vm) / (denominator * denominator);
}

// ray sampling functions

// generate normal vector from microfacet model
// n: normal vector from mesh, alpha: alpha value of material (roughness^2)
template<class RandGen>
vec3f32 sample_ggx(const vec3f32& n, f32 alpha, RandGen& rng) {
    std::uniform_real_distribution<f32> rand_val{};

    auto alpha2 = alpha * alpha;

    auto r1 = rand_val(rng);
    auto r2 = rand_val(rng);

    auto phi = 2.0f * F32_PI * r1;
    auto cos_theta = std::sqrt((1.0f - r2) / (1.0001f + (alpha2 - 1.0f) * r2));
    auto sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);
    auto x = sin_theta * std::cos(phi);
    auto y = sin_theta * std::sin(phi);
    auto z = cos_theta;

    auto up = (std::abs(n.y) < 0.99f) ? vec3f32(0.0f, 1.0f, 0.0f) : vec3f32(1.0f, 0.0f, 0.0f);
    auto t = normalize(cross(up, n));
    auto s = cross(n, t);

    return normalize(s * x + t * y + n * z);
}

template<class RandGen>
std::pair<vec3f32, vec3f32> sample_brdf(const vec3f32& i, const vec3f32& n, f32 alpha, f32 n1, f32 n2, RandGen& rng) {
    auto m = sample_ggx(n, alpha, rng);

    return { normalize(reflect(i, m)), m };
}

template<class RandGen> 
std::tuple<vec3f32, vec3f32, bool> sample_btdf(const vec3f32& i, const vec3f32& n, f32 alpha, f32 n1, f32 n2, RandGen& rng) {
    auto m = sample_ggx(n, alpha, rng);

    auto refracted = refract(i, m, n1, n2);
    if(refracted) {
        return { normalize(*refracted), m, true };
    }
    else {
        return { normalize(reflect(i, m)), m, false };
    }
}

template<class RandGen>
std::tuple<vec3f32, vec3f32, f32> sample_bsdf(const vec3f32& i, const vec3f32& n, f32 roughness, f32 n1, f32 n2, RandGen& rng) {
    std::uniform_real_distribution rand_val{};

    auto alpha = roughness * roughness;

    auto tmp_albedo = vec3f32(0.0f, 0.5f, 1.0f);

    auto cos_theta = dot(-i, n);
    auto f0 = f0_(n1, n2);
    auto reflectance = fresnel_(f0, cos_theta);

    // BRDF
    if(rand_val(rng) <= reflectance) {
        auto [l, m] = sample_brdf(i, n, roughness, n1, n2, rng);
        auto bsdf_value = brdf(l, i, n, m, tmp_albedo, alpha, n1, n2);
        auto pdf_value = pdf_brdf(l, i, n, m, alpha, n1, n2);

        return { l, bsdf_value, pdf_value };
    }
    // BTDF
    else {
        auto [l, m, is_refracted] = sample_btdf(i, n, roughness, n1, n2, rng);
        // refracted -> BTDF
        if(is_refracted) {
            auto bsdf_value = btdf(l, i, n, m, tmp_albedo, alpha, n1, n2);
            auto pdf_value = pdf_btdf(l, i, n, m, alpha, n1, n2);

            return { l, bsdf_value, pdf_value };
        }
        // relected -> BRDF
        else {
            auto bsdf_value = brdf(l, i, n, m, tmp_albedo, alpha, n1, n2);
            auto pdf_value = pdf_brdf(l, i, n, m, alpha, n1, n2);

            return { l, bsdf_value, pdf_value };
        }
    }
}

}
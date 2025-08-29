#pragma once

#include <random>

#include "vector.hpp"

namespace lumina {

// from google gemini
// provisionally implementation

// n1: incomming refrective index, n2: outgoing refrective index
f32 f0_(f32 n1, f32 n2) {
    auto f = (n1 - n2) / (n1 + n2);
    return f * f;
}

// schlick's approximation
// f0: fresnel's reflection coefficient, cos_theta: dot product between l and h
f32 fresnel_(f32 f0, f32 cos_theta) {
    return f0 + (1.0f - f0) * std::pow(1.0f - cos_theta, 5.0f);
}

f32 ndf_ggx_(const vec3f32& n, const vec3f32& m, f32 a) {
    auto a2 = a * a;
    auto nm = dot(n, m);
    auto denominator = nm * nm * (a2 - 1.0f) + 1.0f;
    return a2 / (F32_PI * denominator * denominator);
}

// cook-torrance model
// l: vector to light, v: vector to eye, n: normal vector, m: microfacet normal vector, albedo: color of diffuse component
// roughness: roughness of specular reflection, n1: incoming reflective index [unused], n2: outgoing reflective index [unused]
vec3f32 brdf(const vec3f32& l, const vec3f32& v, const vec3f32& n, const vec3f32& m, const vec3f32& albedo, f32 roughness, f32 n1, f32 n2) {
    auto h = normalize(l + v);

    // fresnel's term
    auto f0 = f0_(n1, n2);
    auto f = fresnel_(f0, dot(l, h));

    // normal distribution function
    // GGX (generalized Trowbridge-Reitz) distribution
    auto alpha = roughness * roughness;
    auto d_ggx = ndf_ggx_(n, m, alpha);

    // geometry term
    auto g_smith = [](f32 nv, f32 alpha) {
        auto k = alpha * 0.5f;
        return nv / (nv * (1.0f - k) + k);
    };
    auto g_ggx = g_smith(dot(n, l), alpha) * g_smith(dot(n, v), alpha);

    auto specular = vec3f32(f * g_ggx * d_ggx / (4.0f * dot(n, l) * dot(n, v)));

    // diffuse term
    auto diffuse = albedo / F32_PI;

    return specular + diffuse;
}

f32 pdf_brdf(const vec3f32& l, const vec3f32& v, const vec3f32& n, const vec3f32& m, f32 alpha) {
    auto nm = std::abs(dot(n, m));
    auto omega = std::abs(dot(v, m));

    auto d = ndf_ggx_(n, m, alpha);

    return (d * nm) / (4.0f * omega);
}

// l: vector to light, v: vector to eye, n: normal vector, m: microfacet normal [unused], albedo: color of diffuse component [unused]
// roughness: roughness of specular reflection [unused], n1: incoming reflective index, n2: outgoing reflective index
vec3f32 btdf(const vec3f32& l, const vec3f32& v, const vec3f32& n, const vec3f32& m, const vec3f32& albedo, f32 roughness, f32 n1, f32 n2) {
    auto nl = dot(n, l);

    auto refracted = refract(l, n, n1, n2);

    auto cos_theta_1 = std::abs(nl);
    auto cos_theta_2 = std::abs(dot(n, *refracted));

    auto denominator = n2 * n2 * cos_theta_1 / (n1 * n1 * cos_theta_2);

    return vec3f32(denominator / (4.0f * cos_theta_1));
}

f32 pdf_btdf(const vec3f32& l, const vec3f32& v, const vec3f32& n, const vec3f32& m, f32 alpha, f32 n1, f32 n2) {
    auto d = ndf_ggx_(n, m, alpha);

    auto n1_n2 = n1 / n2;
    auto n2_n1 = n2 / n1;

    auto lm = std::abs(dot(l, m));
    auto vm = std::abs(dot(v, m));

    auto denominator = (n1_n2 * vm + n2_n1 * lm);
    return d * lm * vm / (denominator * denominator);
}

// generate normal vector from microfacet model
// n: normal vector from mesh, roughness: roughness of material
template<class RandGen>
vec3f32 sample_ggx(const vec3f32& n, f32 roughness, RandGen& rng) {
    auto alpha = roughness * roughness;
    auto alpha2 = alpha * alpha;

    std::uniform_real_distribution<f32> rand_val{};

    auto r1 = rand_val(rng);
    auto r2 = rand_val(rng);

    auto phi = 2.0f * F32_PI * r1;
    auto cos_theta = std::sqrt((1.0f - r2) / (1.0f + (alpha2 - 1.0f) * r2));
    auto sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);

    auto h = vec3f32(sin_theta * std::cos(phi), sin_theta * std::sin(phi), cos_theta);

    auto up = (std::abs(n.y) < 0.99f) ? vec3f32(0.0f, 1.0f, 0.0f) : vec3f32(1.0f, 0.0f, 0.0f);
    
    auto t = normalize(cross(up, n));
    auto b = cross(n, t);

    return normalize(t * h.x + b * h.y + n * h.z);
}

template<class RandGen>
vec3f32 sample_brdf(const vec3f32& i, const vec3f32& n, f32 roughness, f32 n1, f32 n2, RandGen& rng) {
    auto m = sample_ggx(n, roughness, rng);

    return normalize(reflect(i, m));
}

template<class RandGen> 
vec3f32 sample_btdf(const vec3f32& i, const vec3f32& n, f32 roughness, f32 n1, f32 n2, RandGen& rng) {
    auto m = sample_ggx(n, roughness, rng);

    auto refracted = refract(i, m, n1, n2);
    if(refracted) {
        return *refracted;
    }
    else {
        reflect(i, h);
    }
}

template<class RandGen>
vec3f32 sample_bsdf(const vec3f32& i, const vec3f32& n, f32 roughness, f32 n1, f32 n2, RandGen& rng) {
    std::uniform_real_distribution rand_val{};

    auto cos_theta = dot(-i, n);
    auto f0 = f0_(n1, n2);
    auto reflectance = fresnel_(f0, cos_theta);

    // BRDF
    if(rand_val(rng) < reflectance) {
        return sample_brdf(i, n, roughness, n1, n2, rng);
    }
    // BTDF
    else {
        return sample_btdf(i, n, roughness, n1, n2, rng);
    }
}

}
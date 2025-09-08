#pragma once

#include <format>
#include <optional>
#include <ostream>

#include <cmath>

#include "base.hpp"
#include "rng.hpp"

namespace lumina {

// 2D vector class
template<typename T>
union vec2 {
private:
    T arr_[2];
public:
    struct {
        T x;
        T y;
    };
    struct {
        T u;
        T v;
    };
    struct {
        T s;
        T t;
    };

    constexpr vec2() noexcept : arr_{0, 0} {}
    constexpr vec2(T v) noexcept : arr_{v, v} {}
    constexpr vec2(T x, T y) noexcept : arr_{x, y} {}

    // subscription
    constexpr const T& operator[](size_t i) const& noexcept { return arr_[i]; }
    constexpr T& operator[](size_t i) & noexcept { return arr_[i]; }
    constexpr T operator[](size_t i) const&& noexcept { return arr_[i]; }

    // unary operations
    constexpr vec2<T> operator+() const noexcept { return { arr_[0],  arr_[1] }; }
    constexpr vec2<T> operator-() const noexcept { return {-arr_[0], -arr_[1] }; }

    // compound operations
    constexpr vec2<T>& operator+=(const vec2<T>& v) noexcept {
        arr_[0] += v.arr_[0];
        arr_[1] += v.arr_[1];
        return *this;
    }
    constexpr vec2<T>& operator-=(const vec2<T>& v) noexcept {
        arr_[0] -= v.arr_[0];
        arr_[1] -= v.arr_[1];
        return *this;
    }
    constexpr vec2<T>& operator*=(T s) noexcept {
        arr_[0] *= s;
        arr_[1] *= s;
        return *this;
    }
    // element-wise multiplication
    constexpr vec2<T>& operator*=(const vec2<T>& v) noexcept {
        arr_[0] *= v.arr_[0];
        arr_[1] *= v.arr_[1];
        return *this;
    }
    constexpr vec2<T>& operator/=(T s) noexcept {
        arr_[0] /= s;
        arr_[1] /= s;
        return *this;
    }
    // element-wise division
    constexpr vec2<T>& operator/=(const vec2<T>& v) noexcept {
        arr_[0] /= v.arr_[0];
        arr_[1] /= v.arr_[1];
        return *this;
    }

    // vector specific operations
    constexpr T dot(const vec2<T>& v) const noexcept {
        return x * v.x + y * v.y;
    }
    constexpr T norm() const noexcept {
        return std::sqrt(dot(*this));
    }
};

// binary operations
template<typename T>
inline constexpr vec2<T> operator+(const vec2<T>& a, const vec2<T>& b) noexcept {
    return vec2<T>(a) += b;
}
template<typename T>
inline constexpr vec2<T> operator-(const vec2<T>& a, const vec2<T>& b) noexcept {
    return vec2<T>(a) -= b;
}
template<typename T>
inline constexpr vec2<T> operator*(const vec2<T>& v, T s) noexcept {
    return vec2<T>(v) *= s;
}
template<typename T>
inline constexpr vec2<T> operator*(T s, const vec2<T>& v) noexcept {
    return vec2<T>(v) *= s;
}
// element-wise multiplication
template<typename T>
inline constexpr vec2<T> operator*(const vec2<T>& a, const vec2<T>& b) noexcept {
    return vec2<T>(a) *= b;
}
template<typename T>
inline constexpr vec2<T> operator/(const vec2<T>& v, T s) noexcept {
    return vec2<T>(v) /= s;
}
// element-wise division
template<typename T>
inline constexpr vec2<T> operator/(const vec2<T>& a, const vec2<T>& b) noexcept {
    return vec2<T>(a) /= b;
}

template<typename T>
inline constexpr vec2<T> min(const vec2<T>& a, const vec2<T>& b) noexcept {
    return { std::min(a.x, b.x), std::min(a.y, b.y) };
}

template<typename T>
inline constexpr vec2<T> max(const vec2<T>& a, const vec2<T>& b) noexcept {
    return { std::max(a.x, b.x), std::max(a.y, b.y) };
}

// vector specific operations
template<typename T>
inline constexpr T dot(const vec2<T>& a, const vec2<T>& b) noexcept {
    return a.dot(b);
}
template<typename T>
inline constexpr T norm(const vec2<T>& v) noexcept {
    return v.norm();
}
template<typename T>
inline constexpr vec2<T> normalize(const vec2<T>& v) noexcept {
    return v / v.norm();
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const vec2<T>& v) {
    if constexpr(std::is_floating_point_v<T>) {
        os << std::format("({:.2f}, {:.2f})", v.x, v.y);
    }
    else {
        os << std::format("({}, {})", v.x, v.y);
    }
    return os;
}

using vec2s8  =  vec2<s8>;
using vec2s16 = vec2<s16>;
using vec2s32 = vec2<s32>;
using vec2s64 = vec2<s64>;

using vec2u8  =  vec2<u8>;
using vec2u16 = vec2<u16>;
using vec2u32 = vec2<u32>;
using vec2u64 = vec2<u64>;

using vec2f32 = vec2<f32>;
using vec2f64 = vec2<f64>;

// 3D vector class
template<typename T>
union vec3 {
private:
    T arr_[3];
public:
    struct {
        T x;
        T y;
        T z;
    };
    struct {
        T r;
        T g;
        T b;
    };
    struct {
        T u;
        T v;
        T w;
    };

    constexpr vec3() noexcept : arr_{0, 0, 0} {}
    constexpr vec3(T v) noexcept : arr_{v, v, v} {}
    constexpr vec3(T x, T y, T z) noexcept : arr_{x, y, z} {}

    // subscription
    constexpr const T& operator[](size_t i) const& noexcept { return arr_[i]; }
    constexpr T& operator[](size_t i) & noexcept { return arr_[i]; }
    constexpr T operator[](size_t i) const&& noexcept { return arr_[i]; }

    // unary operations
    constexpr vec3<T> operator+() const noexcept { return { arr_[0],  arr_[1],  arr_[2]}; }
    constexpr vec3<T> operator-() const noexcept { return {-arr_[0], -arr_[1], -arr_[2]}; }

    // compound operations
    constexpr vec3<T>& operator+=(const vec3<T>& v) noexcept {
        arr_[0] += v.arr_[0];
        arr_[1] += v.arr_[1];
        arr_[2] += v.arr_[2];
        return *this;
    }
    constexpr vec3<T>& operator-=(const vec3<T>& v) noexcept {
        arr_[0] -= v.arr_[0];
        arr_[1] -= v.arr_[1];
        arr_[2] -= v.arr_[2];
        return *this;
    }
    constexpr vec3<T>& operator*=(T s) noexcept {
        arr_[0] *= s;
        arr_[1] *= s;
        arr_[2] *= s;
        return *this;
    }
    // element-wise multiplication
    constexpr vec3<T>& operator*=(const vec3<T>& v) noexcept {
        arr_[0] *= v.arr_[0];
        arr_[1] *= v.arr_[1];
        arr_[2] *= v.arr_[2];
        return *this;
    }
    constexpr vec3<T>& operator/=(T s) noexcept {
        arr_[0] /= s;
        arr_[1] /= s;
        arr_[2] /= s;
        return *this;
    }
    // element-wise division
    constexpr vec3<T>& operator/=(const vec3<T>& v) noexcept {
        arr_[0] /= v.arr_[0];
        arr_[1] /= v.arr_[1];
        arr_[2] /= v.arr_[2];
        return *this;
    }

    // vector specific operations
    constexpr T dot(const vec3<T>& v) const noexcept {
        return x * v.x + y * v.y + z * v.z;
    }
    constexpr vec3<T> cross(const vec3<T>& v) const noexcept {
        return vec3<T>(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
    constexpr T norm() const noexcept {
        return std::sqrt(dot(*this));
    }
};

// binary operations
template<typename T>
inline constexpr vec3<T> operator+(const vec3<T>& a, const vec3<T>& b) noexcept {
    return vec3<T>(a) += b;
}
template<typename T>
inline constexpr vec3<T> operator-(const vec3<T>& a, const vec3<T>& b) noexcept {
    return vec3<T>(a) -= b;
}
template<typename T>
inline constexpr vec3<T> operator*(const vec3<T>& v, T s) noexcept {
    return vec3<T>(v) *= s;
}
template<typename T>
inline constexpr vec3<T> operator*(T s, const vec3<T>& v) noexcept {
    return vec3<T>(v) *= s;
}
// element-wise multiplication
template<typename T>
inline constexpr vec3<T> operator*(const vec3<T>& a, const vec3<T>& b) noexcept {
    return vec3<T>(a) *= b;
}
template<typename T>
inline constexpr vec3<T> operator/(const vec3<T>& v, T s) noexcept {
    return vec3<T>(v) /= s;
}
// element-wise division
template<typename T>
inline constexpr vec3<T> operator/(const vec3<T>& a, const vec3<T>& b) noexcept {
    return vec3<T>(a) /= b;
}

template<typename T>
inline constexpr vec3<T> min(const vec3<T>& a, const vec3<T>& b) noexcept {
    return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}

template<typename T>
inline constexpr vec3<T> max(const vec3<T>& a, const vec3<T>& b) noexcept {
    return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}

// vector specific operations
template<typename T>
inline constexpr T dot(const vec3<T>& a, const vec3<T>& b) noexcept {
    return a.dot(b);
}
template<typename T>
inline constexpr vec3<T> cross(const vec3<T>& a, const vec3<T>& b) noexcept {
    return a.cross(b);
}
template<typename T>
inline constexpr T norm(const vec3<T>& v) noexcept {
    return v.norm();
}
template<typename T>
inline constexpr vec3<T> normalize(const vec3<T>& v) noexcept {
    return v / v.norm();
}

template<typename T>
inline constexpr T distance(const vec3<T>& a, const vec3<T>& b) noexcept {
    return (a - b).norm();
}

template<typename T>
inline constexpr vec3<T> reflect(const vec3<T>& i, const vec3<T>& n) {
    return i - 2 * dot(i, n) * n;
}

template<typename T>
inline constexpr std::optional<vec3<T>> refract(const vec3<T>& i, const vec3<T>& n, f32 n1, f32 n2) {
    auto cos_theta1 = dot(-i, n);
    auto n_ratio = n1 / n2;
    auto sin_theta2_sq = n_ratio * n_ratio * (1 - cos_theta1 * cos_theta1);

    if(sin_theta2_sq > 1.0f) {
        return std::nullopt;
    }

    auto cos_theta2 = std::sqrt(1 - sin_theta2_sq);
    return n_ratio * i + (n_ratio * cos_theta1 - cos_theta2) * n;
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const vec3<T>& v) {
    if constexpr(std::is_floating_point_v<T>) {
        os << std::format("({:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z);
    }
    else {
        os << std::format("({}, {}, {})", v.x, v.y, v.z);
    }
    return os;
}

using vec3s8  =  vec3<s8>;
using vec3s16 = vec3<s16>;
using vec3s32 = vec3<s32>;
using vec3s64 = vec3<s64>;

using vec3u8  =  vec3<u8>;
using vec3u16 = vec3<u16>;
using vec3u32 = vec3<u32>;
using vec3u64 = vec3<u64>;

using vec3f32 = vec3<f32>;
using vec3f64 = vec3<f64>;

// vector n should be normalized
template<class RandGen>
inline vec3f32 sample_uniform_hemisphere(const vec3f32& n, RandGen& rng) {
    std::uniform_real_distribution<f32> r{};

    auto r1 = r(rng);
    auto r2 = r(rng);
    auto phi = 2.0f * F32_PI * r1;
    auto sin_theta = std::sqrt(1.0f - r2 * r2);
    auto x = sin_theta * std::cos(phi);
    auto y = sin_theta * std::sin(phi);
    auto z = r2;

    vec3f32 axis = std::abs(n.x) > 0.001f ? vec3f32(0.0f, 1.0f, 0.0f) : vec3f32(1.0f, 0.0f, 0.0f);
    auto t = normalize(cross(axis, n));
    auto s = cross(n, t);

    return normalize(s * x + t * y + n * z);
}

// vector n should be normalized
template<class RandGen>
inline vec3f32 sample_cosine_hemisphere(const vec3f32& n, RandGen& rng) {
    std::uniform_real_distribution<f32> r{};

    auto r1 = r(rng);
    auto r2 = r(rng);
    auto phi = 2.0f * F32_PI * r1;
    auto x = std::cos(phi) * std::sqrt(r2);
    auto y = std::sin(phi) * std::sqrt(r2);
    auto z = std::sqrt(1.0f - r2);

    vec3f32 axis = std::abs(n.x) > 0.001f ? vec3f32(0.0f, 1.0f, 0.0f) : vec3f32(1.0f, 0.0f, 0.0f);
    auto t = normalize(cross(axis, n));
    auto s = cross(n, t);

    return normalize(s * x + t * y + n * z);
}

// 4D vector class
template<typename T>
union vec4 {
private:
    T arr_[4];
public:
    struct {
        T x;
        T y;
        T z;
        T w;
    };
    struct {
        T r;
        T g;
        T b;
        T a;
    };

    constexpr vec4() noexcept : arr_{0, 0, 0, 0} {}
    constexpr vec4(T v) noexcept : arr_{v, v, v, v} {}
    constexpr vec4(T x, T y, T z, T w) noexcept : arr_{x, y, z, w} {}

    // subscription
    constexpr const T& operator[](size_t i) const& noexcept { return arr_[i]; }
    constexpr T& operator[](size_t i) & noexcept { return arr_[i]; }
    constexpr T operator[](size_t i) const&& noexcept { return arr_[i]; }

    // unary operations
    constexpr vec4<T> operator+() const noexcept { return { arr_[0],  arr_[1],  arr_[2],  arr_[3] }; }
    constexpr vec4<T> operator-() const noexcept { return {-arr_[0], -arr_[1], -arr_[2], -arr_[3] }; }

    // compound operations
    constexpr vec4<T>& operator+=(const vec4<T>& v) noexcept {
        arr_[0] += v.arr_[0];
        arr_[1] += v.arr_[1];
        arr_[2] += v.arr_[2];
        arr_[3] += v.arr_[3];
        return *this;
    }
    constexpr vec4<T>& operator-=(const vec4<T>& v) noexcept {
        arr_[0] -= v.arr_[0];
        arr_[1] -= v.arr_[1];
        arr_[2] -= v.arr_[2];
        arr_[3] -= v.arr_[3];
        return *this;
    }
    constexpr vec4<T>& operator*=(T s) noexcept {
        arr_[0] *= s;
        arr_[1] *= s;
        arr_[2] *= s;
        arr_[3] *= s;
        return *this;
    }
    // element-wise multiplication
    constexpr vec4<T>& operator*=(const vec4<T>& v) noexcept {
        arr_[0] *= v.arr_[0];
        arr_[1] *= v.arr_[1];
        arr_[2] *= v.arr_[2];
        arr_[3] *= v.arr_[3];
        return *this;
    }
    constexpr vec4<T>& operator/=(T s) noexcept {
        arr_[0] /= s;
        arr_[1] /= s;
        arr_[2] /= s;
        arr_[3] /= s;
        return *this;
    }
    // element-wise division
    constexpr vec4<T>& operator/=(const vec4<T>& v) noexcept {
        arr_[0] /= v.arr_[0];
        arr_[1] /= v.arr_[1];
        arr_[2] /= v.arr_[2];
        arr_[3] /= v.arr_[3];
        return *this;
    }

    // vector specific operations
    constexpr T dot(const vec4<T>& v) const noexcept {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }
    constexpr T norm() const noexcept {
        return std::sqrt(dot(*this));
    }
};

// binary operations
template<typename T>
inline constexpr vec4<T> operator+(const vec4<T>& a, const vec4<T>& b) noexcept {
    return vec4<T>(a) += b;
}
template<typename T>
inline constexpr vec4<T> operator-(const vec4<T>& a, const vec4<T>& b) noexcept {
    return vec4<T>(a) -= b;
}
template<typename T>
inline constexpr vec4<T> operator*(const vec4<T>& v, T s) noexcept {
    return vec4<T>(v) *= s;
}
template<typename T>
inline constexpr vec4<T> operator*(T s, const vec4<T>& v) noexcept {
    return vec4<T>(v) *= s;
}
// element-wise multiplication
template<typename T>
inline constexpr vec4<T> operator*(const vec4<T>& a, const vec4<T>& b) noexcept {
    return vec4<T>(a) *= b;
}
template<typename T>
inline constexpr vec4<T> operator/(const vec4<T>& v, T s) noexcept {
    return vec4<T>(v) /= s;
}
// element-wise division
template<typename T>
inline constexpr vec4<T> operator/(const vec4<T>& a, const vec4<T>& b) noexcept {
    return vec4<T>(a) /= b;
}

template<typename T>
inline constexpr vec4<T> min(const vec4<T>& a, const vec4<T>& b) noexcept {
    return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w) };
}

template<typename T>
inline constexpr vec4<T> max(const vec4<T>& a, const vec4<T>& b) noexcept {
    return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) };
}

// vector specific operations
template<typename T>
inline constexpr T dot(const vec4<T>& a, const vec4<T>& b) noexcept {
    return a.dot(b);
}
template<typename T>
inline constexpr T norm(const vec4<T>& v) noexcept {
    return v.norm();
}
template<typename T>
inline constexpr vec4<T> normalize(const vec4<T>& v) noexcept {
    return v / v.norm();
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const vec4<T>& v) {
    if constexpr(std::is_floating_point_v<T>) {
        os << std::format("({:.2f}, {:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z, v.w);
    }
    else {
        os << std::format("({}, {}, {}, {})", v.x, v.y, v.z, v.w);
    }
    return os;
}

using vec4s8  =  vec4<s8>;
using vec4s16 = vec4<s16>;
using vec4s32 = vec4<s32>;
using vec4s64 = vec4<s64>;

using vec4u8  =  vec4<u8>;
using vec4u16 = vec4<u16>;
using vec4u32 = vec4<u32>;
using vec4u64 = vec4<u64>;

using vec4f32 = vec4<f32>;
using vec4f64 = vec4<f64>;

}

template<typename T>
struct std::formatter<lumina::vec2<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::vec2<T>& v, std::format_context& ctx) const {
        if constexpr(std::is_floating_point_v<T>) {
            return std::format_to(ctx.out(), "({:.2f}, {:.2f})", v.x, v.y);
        }
        else {
            return std::format_to(ctx.out(), "({}, {})", v.x, v.y);
        }
    }
};

template<typename T>
struct std::formatter<lumina::vec3<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::vec3<T>& v, std::format_context& ctx) const {
        if constexpr(std::is_floating_point_v<T>) {
            return std::format_to(ctx.out(), "({:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z);
        }
        else {
            return std::format_to(ctx.out(), "({}, {}, {})", v.x, v.y, v.z);
        }
    }
};

template<typename T>
struct std::formatter<lumina::vec4<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto iter = ctx.begin();
        return iter;
    }

    auto format(const lumina::vec4<T>& v, std::format_context& ctx) const {
        if constexpr(std::is_floating_point_v<T>) {
            return std::format_to(ctx.out(), "({:.2f}, {:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z, v.w);
        }
        else {
            return std::format_to(ctx.out(), "({}, {}, {}, {})", v.x, v.y, v.z, v.w);
        }
    }
};
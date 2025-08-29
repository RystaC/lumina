#pragma once

#include "base.hpp"
#include "vector.hpp"

namespace lumina {

template<typename T>
union mat4x4 {
private:
    T arr_[16];
public:
    struct {
        T e00, e10, e20, e30;
        T e01, e11, e21, e31;
        T e02, e12, e22, e32;
        T e03, e13, e23, e33;
    };

    constexpr mat4x4() noexcept : arr_{
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    } {}

    // matrix-like subscription
    const T& operator[](size_t r, size_t c) const& { return arr_[c * 4 + r]; }
    T& operator[](size_t r, size_t c) & { return arr_[c * 4 + r]; }
    T operator[](size_t r, size_t c) const&& { return arr_[c * 4 + r]; }
};

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const mat4x4<T>& m) {
    os << std::format(
R"(   c0 c1 c2 c3
r0 {} {} {} {}
r1 {} {} {} {}
r2 {} {} {} {}
r3 {} {} {} {})",
        m.e00, m.e01, m.e02, m.e03,
        m.e10, m.e11, m.e12, m.e13,
        m.e20, m.e21, m.e22, m.e23,
        m.e30, m.e31, m.e32, m.e33
    );
    return os;
}

using mat4x4f32 = mat4x4<f32>;

}
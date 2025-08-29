#pragma once

#include <iostream>
#include <limits>
#include <numbers>

#include <cstdint>

namespace lumina {

// signed integer types
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

// unsigned integer types
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// floating point types
using f32 = float;
using f64 = double;

// address space type
using usize = size_t;

// min, max values
constexpr  s8  S8_MIN =  std::numeric_limits<s8>::min();
constexpr  s8  S8_MAX =  std::numeric_limits<s8>::max();
constexpr s16 S16_MIN = std::numeric_limits<s16>::min();
constexpr s16 S16_MAX = std::numeric_limits<s16>::max();
constexpr s32 S32_MIN = std::numeric_limits<s32>::min();
constexpr s32 S32_MAX = std::numeric_limits<s32>::max();
constexpr s64 S64_MIN = std::numeric_limits<s64>::min();
constexpr s64 S64_MAX = std::numeric_limits<s64>::max();

constexpr  u8  U8_MIN =  std::numeric_limits<u8>::min();
constexpr  u8  U8_MAX =  std::numeric_limits<u8>::max();
constexpr u16 U16_MIN = std::numeric_limits<u16>::min();
constexpr u16 U16_MAX = std::numeric_limits<u16>::max();
constexpr u32 U32_MIN = std::numeric_limits<u32>::min();
constexpr u32 U32_MAX = std::numeric_limits<u32>::max();
constexpr u64 U64_MIN = std::numeric_limits<u64>::min();
constexpr u64 U64_MAX = std::numeric_limits<u64>::max();

constexpr f32 F32_MIN = std::numeric_limits<f32>::lowest();
constexpr f32 F32_MAX =    std::numeric_limits<f32>::max();
constexpr f64 F64_MIN = std::numeric_limits<f64>::lowest();
constexpr f64 F64_MAX =    std::numeric_limits<f64>::max();

// floating-point specific values
constexpr f32   F32_NEXT_ZERO =     std::numeric_limits<f32>::min();
constexpr f64   F64_NEXT_ZERO =     std::numeric_limits<f64>::min();
constexpr f32 F32_MACHINE_EPS = std::numeric_limits<f32>::epsilon();
constexpr f64 F64_MACHINE_EPS = std::numeric_limits<f64>::epsilon();

// mathematical values
constexpr f32 F32_PI = std::numbers::pi_v<f32>;
constexpr f64 F64_PI = std::numbers::pi_v<f64>;

// for radians-degrees conversion
constexpr f32 F32_PI_180 = F32_PI / 180.0f;
constexpr f32 F32_180_PI = 180.0f / F32_PI;
constexpr f64 F64_PI_180 =  F64_PI / 180.0;
constexpr f64 F64_180_PI =  180.0 / F64_PI;

inline constexpr f32 radian(f32 degree) {
    return degree * F32_PI_180;
}

inline constexpr f64 radian(f64 degree) {
    return degree * F64_PI_180;
}

inline constexpr f32 degree(f32 radian) {
    return radian * F32_180_PI;
}

inline constexpr f64 degree(f64 radian) {
    return radian * F64_180_PI;
}

}
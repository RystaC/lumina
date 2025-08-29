#pragma once

#include <bit>
#include <random>

#include "base.hpp"

namespace lumina {
// these RNGs referenced from: https://prng.di.unimi.it

namespace internal_ {

// for seed generation
// reference implementation: https://prng.di.unimi.it/splitmix64.c
class splitmix64_ {
    u64 x_;

public:
    constexpr splitmix64_(u64 seed) noexcept : x_(seed) {}

    constexpr u64 next() noexcept {
        auto z = (x_ += 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }
};

// xoshiro256++
// general purpose RNG
// reference implementation: https://prng.di.unimi.it/xoshiro256plusplus.c
class xoshiro256pp_ {
    u64 s_[4];

public:
    constexpr xoshiro256pp_(u64 seed) noexcept {
        auto init_state = splitmix64_(seed);
        s_[0] = init_state.next();
        s_[1] = init_state.next();
        s_[2] = init_state.next();
        s_[3] = init_state.next();
    }

    constexpr u64 next() noexcept {
        auto result = std::rotl(s_[0] + s_[3], 23) + s_[0];
        auto t = s_[1] << 17;

        s_[2] ^= s_[0];
        s_[3] ^= s_[1];
        s_[1] ^= s_[2];
        s_[0] ^= s_[3];

        s_[2] ^= t;

        s_[3] = std::rotl(s_[3], 45);

        return result;
    }
};

// xoshiro256+
// for floating-point random numbers (faster than xoshiro256++)
// I don't know how to use its result
// reference implementation: https://prng.di.unimi.it/xoshiro256plus.c
class xoshiro256p_ {
    u64 s_[4];

public:
    constexpr xoshiro256p_(u64 seed) noexcept {
        auto init_state = splitmix64_(seed);
        s_[0] = init_state.next();
        s_[1] = init_state.next();
        s_[2] = init_state.next();
        s_[3] = init_state.next();
    }

    constexpr u64 next() noexcept {
        auto result = s_[0] + s_[3];
        auto t = s_[1] << 17;

        s_[2] ^= s_[0];
        s_[3] ^= s_[1];
        s_[1] ^= s_[2];
        s_[0] ^= s_[3];

        s_[2] ^= t;

        s_[3] = std::rotl(s_[3], 45);

        return result;
    }
};

// xoroshiro128++
// small space RNG
// reference implementation: https://prng.di.unimi.it/xoroshiro128plusplus.c
class xoroshiro128pp_ {
    u64 s_[2];

public:
    constexpr xoroshiro128pp_(u64 seed) noexcept {
        auto init_state = splitmix64_(seed);
        s_[0] = init_state.next();
        s_[1] = init_state.next();
    }

    constexpr u64 next() noexcept {
        auto s0 = s_[0];
        auto s1 = s_[1];
        auto result = std::rotl(s0 + s1, 17) + s0;
        s1 ^= s0;
        s_[0] = std::rotl(s0, 49) ^ s1 ^ (s1 << 21);
        s_[1] = std::rotl(s1, 28);

        return result;
    }
};

// for std::uniform_[real|int]_distribution
template<class RandGen>
class rng_base_ {
    RandGen rand_gen_;

public:
    using result_type = u64;

    constexpr rng_base_(u64 seed) noexcept : rand_gen_(seed) {}
    
    static constexpr result_type min() noexcept {
        return std::numeric_limits<result_type>::min();
    }

    static constexpr result_type max() noexcept {
        return std::numeric_limits<result_type>::max();
    }

    result_type operator()() {
        return rand_gen_.next();
    }
};

}

using xoshiro256pp   = internal_::rng_base_<internal_::xoshiro256pp_>;
using xoshiro256p    = internal_::rng_base_<internal_::xoshiro256p_>;
using xoroshiro128pp = internal_::rng_base_<internal_::xoroshiro128pp_>;

}
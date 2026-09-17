#pragma once
#include <cstdint>

using f32 = float;
using f64 = double;

static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

template<typename T>
struct Tvec2
{
    T x;
    T y;
};


template<typename T>
struct Tvec3
{
    T x;
    T y;
    T z;
};


template<typename T>
struct Tvec4
{
    T x;
    T y;
    T z;
    T w;
};

using vec2 = Tvec2<f32>;
using vec3 = Tvec3<f32>;
using vec4 = Tvec4<f32>;

using ivec2 = Tvec2<i32>;
using ivec3 = Tvec3<i32>;
using ivec4 = Tvec4<i32>;

using i8vec2 = Tvec2<i8>;
using i8vec3 = Tvec3<i8>;
using i8vec4 = Tvec4<i8>;

using i16vec2 = Tvec2<i16>;
using i16vec3 = Tvec3<i16>;
using i16vec4 = Tvec4<i16>;

using i32vec2 = Tvec2<i32>;
using i32vec3 = Tvec3<i32>;
using i32vec4 = Tvec4<i32>;

using i64vec2 = Tvec2<i64>;
using i64vec3 = Tvec3<i64>;
using i64vec4 = Tvec4<i64>;

using uvec2 = Tvec2<u32>;
using uvec3 = Tvec3<u32>;
using uvec4 = Tvec4<u32>;

using u8vec2 = Tvec2<u8>;
using u8vec3 = Tvec3<u8>;
using u8vec4 = Tvec4<u8>;

using u16vec2 = Tvec2<u16>;
using u16vec3 = Tvec3<u16>;
using u16vec4 = Tvec4<u16>;

using u32vec2 = Tvec2<u32>;
using u32vec3 = Tvec3<u32>;
using u32vec4 = Tvec4<u32>;

using u64vec2 = Tvec2<u64>;
using u64vec3 = Tvec3<u64>;
using u64vec4 = Tvec4<u64>;
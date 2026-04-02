/// @ref core
/// @file glm/glm.hpp
///
/// @mainpage OpenGL Mathematics (GLM)
/// - Website: <a href="https://glm.g-truc.net">glm.g-truc.net</a>
/// - <a href="modules.html">GLM API documentation</a>
/// - <a href="https://github.com/g-truc/glm/blob/master/manual.md">GLM Manual</a>
///
/// @defgroup core Core features
///
/// @brief Features that implement in C++ the GLSL specification as closely as possible.
///
/// The GLM core consists of C++ types that mirror GLSL types and
/// C++ functions that mirror the GLSL functions.
///
/// The best documentation for GLM Core is the current GLSL specification,
/// <a href="http://www.opengl.org/registry/doc/GLSLangSpec.4.20.8.clean.pdf">version 4.2
/// (pdf file)</a>.
///
/// GLM core functionalities require <glm/glm.hpp> to be included to be used.
///
///
/// @defgroup core_vector Vector types
///
/// Vector types of two to four components with an exhaustive set of operators.
///
/// @ingroup core
///
///
/// @defgroup core_vector_precision Vector types with precision qualifiers
///
/// @brief Vector types with precision qualifiers which may result in various precision in term of ULPs
///
/// GLSL allows defining qualifiers for particular variables.
/// With OpenGL's GLSL, these qualifiers have no effect; they are there for compatibility,
/// with OpenGL ES's GLSL, these qualifiers do have an effect.
///
/// C++ has no language equivalent to qualifier qualifiers. So GLM provides the next-best thing:
/// a number of typedefs that use a particular qualifier.
///
/// None of these types make any guarantees about the actual qualifier used.
///
/// @ingroup core
///
///
/// @defgroup core_matrix Matrix types
///
/// Matrix types of with C columns and R rows where C and R are values between 2 to 4 included.
/// These types have exhaustive sets of operators.
///
/// @ingroup core
///
///
/// @defgroup core_matrix_precision Matrix types with precision qualifiers
///
/// @brief Matrix types with precision qualifiers which may result in various precision in term of ULPs
///
/// GLSL allows defining qualifiers for particular variables.
/// With OpenGL's GLSL, these qualifiers have no effect; they are there for compatibility,
/// with OpenGL ES's GLSL, these qualifiers do have an effect.
///
/// C++ has no language equivalent to qualifier qualifiers. So GLM provides the next-best thing:
/// a number of typedefs that use a particular qualifier.
///
/// None of these types make any guarantees about the actual qualifier used.
///
/// @ingroup core
///
///
/// @defgroup ext Stable extensions
///
/// @brief Additional features not specified by GLSL specification.
///
/// EXT extensions are fully tested and documented.
///
/// Even if it's highly unrecommended, it's possible to include all the extensions at once by
/// including <glm/ext.hpp>. Otherwise, each extension needs to be included  a specific file.
///
///
/// @defgroup gtc Recommended extensions
///
/// @brief Additional features not specified by GLSL specification.
///
/// GTC extensions aim to be stable with tests and documentation.
///
/// Even if it's highly unrecommended, it's possible to include all the extensions at once by
/// including <glm/ext.hpp>. Otherwise, each extension needs to be included  a specific file.
///
///
/// @defgroup gtx Experimental extensions
///
/// @brief Experimental features not specified by GLSL specification.
///
/// Experimental extensions are useful functions and types, but the development of
/// their API and functionality is not necessarily stable. They can change
/// substantially between versions. Backwards compatibility is not much of an issue
/// for them.
///
/// Even if it's highly unrecommended, it's possible to include all the extensions
/// at once by including <glm/ext.hpp>. Otherwise, each extension needs to be
/// included  a specific file.
///

#include "detail/_fixes.hpp"

#include "detail/setup.hpp"

#pragma once

#include <cmath>
#include <climits>
#include <cfloat>
#include <limits>
#include <cassert>
#include "fwd.hpp"

#include "vec2.hpp"
#include "vec3.hpp"
#include "vec4.hpp"
#include "mat2x2.hpp"
#include "mat2x3.hpp"
#include "mat2x4.hpp"
#include "mat3x2.hpp"
#include "mat3x3.hpp"
#include "mat3x4.hpp"
#include "mat4x2.hpp"
#include "mat4x3.hpp"
#include "mat4x4.hpp"

#include "trigonometric.hpp"
#include "exponential.hpp"
#include "common.hpp"
#include "packing.hpp"
#include "geometric.hpp"
#include "matrix.hpp"
#include "vector_relational.hpp"
#include "integer.hpp"

#define GLM_FORCE_ALIGNED

using f32 = glm::float32;
using f64 = glm::float64;

using i8 = glm::int8;
using i16 = glm::int16;
using i32 = glm::int32;
using i64 = glm::int64;

using u8 = glm::uint8;
using u16 = glm::uint16;
using u32 = glm::uint32;
using u64 = glm::uint64;

using vec2 = glm::fvec2;
using vec3 = glm::fvec3;
using vec4 = glm::fvec4;

using dvec2 = glm::dvec2;
using dvec3 = glm::dvec3;
using dvec4 = glm::dvec4;

using uvec2 = glm::u32vec2;
using uvec3 = glm::u32vec3;
using uvec4 = glm::u32vec4;

using ivec2 = glm::i32vec2;
using ivec3 = glm::i32vec3;
using ivec4 = glm::i32vec4;

using mat2x2 = glm::mat2;
using mat3x3 = glm::mat3;
using mat4x4 = glm::mat4;
using mat2x3 = glm::mat2x3;
using mat3x2 = glm::mat3x2;
using mat2x4 = glm::mat2x4;
using mat4x2 = glm::mat4x2;
using mat3x4 = glm::mat3x4;
using mat4x3 = glm::mat4x3;


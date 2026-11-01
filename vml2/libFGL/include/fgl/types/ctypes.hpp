#pragma once
#ifndef FGL_TYPES_CTYPES_HPP_INCLUDED
#define FGL_TYPES_CTYPES_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

namespace fgl {

/**
@file

@defgroup group-types-ctypes C Type Aliases

@brief c++-style aliases for common C types

@{
*/

/// Used to represent an immutable null-terminated C string
using cstring = const char*;

/// Used to represent a raw byte
using cbyte = unsigned char;

/**
@brief Used to represent a C-style array
@tparam T The type of the array elements
@tparam T_length The number of elements in the array
*/
template <typename T, std::size_t T_length>
using carray = T[T_length];

///@}

} // namespace fgl

#endif // FGL_TYPES_CTYPES_HPP_INCLUDED

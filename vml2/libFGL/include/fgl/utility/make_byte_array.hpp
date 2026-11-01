#pragma once
#ifndef FGL_UTILITY_MAKE_BYTE_ARRAY_HPP_INCLUDED
#define FGL_UTILITY_MAKE_BYTE_ARRAY_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cstddef> // size_t, byte
#include <utility> // index_sequence, make_index_sequence
#include <algorithm> // copy
#include <array>

#include "../types/ctypes.hpp"
#include "../types/string_literal.hpp"

namespace fgl {

/**
@file

@example example/fgl/utility/make_byte_array.cpp
	An example for @ref group-utility-make_byte_array

@defgroup group-utility-make_byte_array Make Byte Array

@brief Makes a <tt>std::array<std::byte></tt> from string literals

@see the example program @ref example/fgl/utility/make_byte_array.cpp
@{
*/

/**
@copybrief group-utility-make_byte_array
@tparam T_length The length of the cstring array; should be deduced
	automatically.
@param cstr The cstring literal
@returns A byte array representation of the string, excluding the
	null-terminator.
*/
template <std::size_t T_length> [[nodiscard]] constexpr
std::array<std::byte, T_length-1> make_byte_array(
	fgl::carray<const char, T_length>& cstr)
{
	constexpr std::size_t arrayLength{ T_length - 1 }; // no null term
	return
		[&]<std::size_t ... T_indexes>(std::index_sequence<T_indexes...>)
		constexpr -> std::array<std::byte, arrayLength>
		{
			return {{static_cast<std::byte>(cstr[T_indexes])...}};
		}(std::make_index_sequence<arrayLength>());
}

} // namespace fgl

///@ingroup group-fgl-literals
namespace fgl::literals {
/**
@brief A literal @c std::array of @c std::byte which represents the string,
	excluding null-terminator.
@see <tt>@ref fgl::make_byte_array()</tt>
*/
template<fgl::string_literal T_string>
consteval auto operator"" _barray()
{
	return make_byte_array(T_string.array);
}

} // namespace fgl::literals

///@} group-utility-make_byte_array

#endif // FGL_UTILITY_MAKE_BYTE_ARRAY_HPP_INCLUDED

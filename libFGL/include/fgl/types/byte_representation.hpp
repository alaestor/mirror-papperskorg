#pragma once
#ifndef FGL_TYPES_BYTE_REPRESENTATION_HPP_INCLUDED
#define FGL_TYPES_BYTE_REPRESENTATION_HPP_INCLUDED

// #include <cstddef> // nullptr_t
// #include <type_traits>
// #include <concepts>
// #include <utility> // forward, move
// #include <stdexcept> // runtime_error, invalid_argument

// #include "../debug/constexpr_assert.hpp"
// #include "./traits.hpp" // null_comparable, pointer_type

#include <array>
#include <algorithm>
#include <bit>
#include <type_traits>
//#include <utility>

namespace fgl {

/**
@file

@example example/fgl/types/byte_representation.cpp
	An example for @ref group-types-byte_representation

@defgroup group-types-byte_representation Byte Representation

@brief

@see the example program @ref example/fgl/types/byte_representation.cpp

@{
*/

/**
@copybrief group-types-byte_representation
@details @parblock
@endparblock
@tparam T The <tt>@ref fgl::traits::null_comparable</tt> type to wrap
*/
template <typename T>
requires std::is_trivially_copyable_v<T>
struct byte_representation : public std::array<std::byte, sizeof(T)>
{
	using base_t = std::array<std::byte, sizeof(T)>;

	[[nodiscard]] constexpr byte_representation(T t) noexcept
	: base_t(std::bit_cast<base_t>(t))
	{}



	constexpr auto operator<=>(const byte_representation&) const noexcept
		= default;

	// T helpers

	// template <typename U>
	// requires std::is_trivially_copyable_v<U> && sizeof(U) == sizeof(T)
	// [[nodiscard]] constexpr U as() noexcept const
	// { return std::bit_cast<U>(static_cast<base_t>(*this)); }

	[[nodiscard]] constexpr T value() const noexcept
	{ return std::bit_cast<T>(static_cast<base_t>(*this)); }

	constexpr T() noexcept const
	{ return value(); }

	constexpr auto operator=(T t) noexcept(noexcept(std::copy<
		typename base_t::const_iterator, typename base_t::iterator>))
	{
		const base_t arr{ std::bit_cast<base_t>(t) };
		std::copy(arr.cbegin(), arr.cend(), this->begin());
	}

	constexpr auto operator<=>(const T& t) const noexcept
	{ return value() <=>  t; }
};

///@}
}; // namespace fgl

#endif // FGL_TYPES_BYTE_REPRESENTATION_HPP_INCLUDED

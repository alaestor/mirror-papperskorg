#pragma once
#ifndef FGL_UTILITY_BITWISE_HPP_INCLUDED
#define FGL_UTILITY_BITWISE_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cstdint> // uint_fast8_t
#include <climits> // CHAR_BIT
#include <concepts> // integral, unsigned_integral
#include <limits> // numeric_limits

namespace fgl {

/**
@file

@defgroup group-utility-bitwise Bitwise

@brief High-performance bitwise operations

@todo support for contiguous byte ranges

@{
*/

/**
@brief Counts the number of set bits (bits that are @c 1 ) in @p bits
@tparam T_out The integral output type. Defaults to @c uint_fast8_t
@param bits An unsigned integral value whose set bits will be counted.
@returns The number of set bits (bits that are @c 1 ) in @p bits
*/
template <std::integral T_out = uint_fast8_t>
[[nodiscard]] constexpr inline
T_out count_set_bits(std::unsigned_integral auto bits) noexcept
{
	using T = decltype(bits);
	constexpr T max{ std::numeric_limits<T>::max() };
	constexpr T pattern1{ max / 3 }; // 0x5555...
	constexpr T pattern2{ max / 15 * 3 }; // 0x3333...
	constexpr T pattern3{ max / 255 * 15 }; // 0xF0F0...
	constexpr T pattern4{ max / 255 }; // 0x1010...

	bits -= (bits >> 1) & pattern1;
	bits = (bits & pattern2) + ((bits >> 2) & pattern2);
	bits = (bits + (bits >> 4)) & pattern3;

	constexpr T highest_to_lowest_byte{ (sizeof(T)-1) * CHAR_BIT };
	return static_cast<T_out>((bits * pattern4) >> highest_to_lowest_byte);
}

/**
@brief Counts the number of unset bits (bits that are @c 0 ) in @p bits
@tparam T_out The integral output type. Defaults to @c uint_fast8_t
@param bits An unsigned integral value whose unset bits will be counted.
@returns The number of set bits (bits that are @c 0 ) in @p bits
*/
template <std::integral T_out = uint_fast8_t>
[[nodiscard]] constexpr inline
T_out count_unset_bits(const std::unsigned_integral auto bits) noexcept
{
	const auto set_bits{ count_set_bits<T_out>(bits) };
	return static_cast<T_out>((sizeof(decltype(bits)) * CHAR_BIT) - set_bits);
}

///@} group-utility-bitwise

} // namespace fgl

#endif // FGL_UTILITY_BITWISE_HPP_INCLUDED

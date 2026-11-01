#pragma once
#ifndef FGL_MATH_DIGITS_HPP_INCLUDED
#define FGL_MATH_DIGITS_HPP_INCLUDED

#include <cstddef> // size_t
#include <cstdint>
#include <concepts>
#include <limits>

namespace fgl {

template <std::unsigned_integral T, uint_fast8_t T_base = 10>
[[nodiscard]] constexpr inline std::size_t digits(const T value)
{
	std::size_t digits{ 1 };
	for (; value > 9; value /= T_base, ++digits);
	return digits;
}

template <std::integral T, uint_fast8_t T_base = 10>
[[nodiscard]] constexpr inline std::size_t digits(const T value)
{
	if (value < 0)
	{
		return 0; // ?
	}
	else
	{
		using u_t = std::make_unsigned_t<T>;
		return digits<u_t, T_base>(static_cast<u_t>(value));
	}
}

// add floating point, etc.

} // namespace fgl


#endif // FGL_MATH_DIGITS_HPP_INCLUDED

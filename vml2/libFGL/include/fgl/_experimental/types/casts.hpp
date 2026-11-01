#pragma once
#ifndef FGL_TYPES_CASTS_HPP_INCLUDED
#define FGL_TYPES_CASTS_HPP_INCLUDED

#include <cstdint> // uintptr_t
#include <bit> // bit_cast
#include <stdexcept> // runtime_error, invalid_argument
#include <limits>
#include <type_traits>
#include <concepts>

#include "../../types/traits.hpp"

namespace fgl {

template <std::signed_integral T> [[nodiscard]] constexpr inline
std::make_unsigned_t<T> strip_sign_cast(const T value) noexcept
{
	return (value >= 0)
		? static_cast<std::make_unsigned_t<T>>(value)
		: static_cast<std::make_unsigned_t<T>>(value * -1);
}

/* Ensures that T_to can represent the value of T_from.
Throws if T_from is outside the [min, max] values of T_to */
template <std::integral T_from, std::integral T_to>
[[nodiscard]] constexpr inline T_to preserve_value_cast(const T_from value)
{
	if (value < std::numeric_limits<T_to>::min())
	{
		throw std::invalid_argument("T cannot represent value (bellow min)");
	}
	else if (value > std::numeric_limits<T_to>::max())
	{
		throw std::invalid_argument("T cannot represent value (above max)");
	}
	else return static_cast<T_to>(value);
}

} // namespace fgl


namespace fgl::i_know_what_im_doing {

// This is as prone to undefined behavior as reinterpret_cast! Use with care!

/* IMPLEMENTATION WARNING: aligned_reinterpret_cast
	bitcast may not accurately represent address alignment on some platforms
	but it should on all modern systems. AFAIK there's no way to check?
*/

// WARNING: not portable, no test
template <fgl::traits::pointer_type T> [[nodiscard]] inline
T aligned_reinterpret_cast_noexcept(const fgl::traits::pointer_type auto ptr)
noexcept
{
	return (std::bit_cast<std::uintptr_t>(ptr) % alignof(T) == 0)
		? reinterpret_cast<T>(reinterpret_cast<void*>(ptr))
		: nullptr;
}

// WARNING: not portable, no test
template <fgl::traits::pointer_type T> [[nodiscard]] inline
T aligned_reinterpret_cast(const fgl::traits::pointer_type auto ptr)
{
	if (const T ret_ptr{ aligned_reinterpret_cast_noexcept<T>(ptr) };
		ret_ptr != nullptr)
	{ return ret_ptr; }
	else throw std::runtime_error("FGL aligned_reinterpret_cast misaligned");
}

} // namespace fgl::i_know_what_im_doing

#endif // FGL_TYPES_CASTS_HPP_INCLUDED

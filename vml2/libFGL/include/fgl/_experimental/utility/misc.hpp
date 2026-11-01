#pragma once
#ifndef FGL_UTILITY_MISC_HPP_INCLUDED
#define FG_UTILITY_MISC_HPP_INCLUDED

#include <type_traits>
#include <memory>

namespace fgl {

///@returns <tt>true</tt> if the value of @p p1 and @p p2 are equal.
[[nodiscard]] constexpr inline bool same_address(
	const void* const p1, const void* const p2) noexcept
{ return p1 == p2; }

template <typename T1, typename T2>
requires (std::is_base_of_v<T1, T2> || std::is_base_of_v<T2, T1>)
[[nodiscard]] constexpr inline bool same_object(
	const T1& lhs, const T2& rhs) noexcept
{ return same_address(std::addressof(lhs), std::addressof(rhs)); }

} // namespace fgl

#endif // FGL_UTILITY_MISC_HPP_INCLUDED

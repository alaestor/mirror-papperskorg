#pragma once
#ifndef FGL_TEST_UTIL_HPP_INCLUDED
#define FGL_TEST_UTIL_HPP_INCLUDED

#include <cstddef> // size_t
#include <array>

#define FGL_SHORT_MACROS

#include <fgl/debug/exception_occurs.hpp>

#define got_expected_exception(expression) \
	assert(specific_exception_occurs(std::invalid_argument, expression))

template
<
    const std::size_t T_depth,
    const std::size_t T_length,
    const std::array<std::size_t, T_length> T_bounds
>
inline constexpr void iterate_bounds_impl(
    std::array<std::size_t, T_length>& indexes,
	const auto& callable)
{
    for (std::size_t& i{ indexes[T_depth] }; i < T_bounds[T_depth]; ++i)
    {
        if constexpr (T_depth < T_length-1)
        {
            iterate_bounds_impl<T_depth + 1, T_length, T_bounds>
				(indexes, callable);
        }
        else
        {
            callable(indexes);
        }
    }
    indexes[T_depth] = 0; // for next iteration
}

template <std::size_t ... T_bounds>
constexpr void iterate_bounds(const auto& callable)
{
    constexpr std::size_t dimensions{ sizeof...(T_bounds) };
    constexpr std::array<std::size_t, dimensions> bounds{ T_bounds... };
    std::array<std::size_t, dimensions> indexes{ 0 };
    iterate_bounds_impl<0, bounds.size(), bounds>(indexes, callable);
}

#endif // FGL_TEST_UTIL_HPP_INCLUDED

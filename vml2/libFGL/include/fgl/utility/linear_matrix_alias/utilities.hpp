#pragma once
#ifndef FGL_UTILITY_LINEAR_MATRIX_ALIAS_UTILITIES_HPP_INCLUDED
#define FGL_UTILITY_LINEAR_MATRIX_ALIAS_UTILITIES_HPP_INCLUDED
#include "../../environment/libfgl_compatibility_check.hpp"

#include <cstddef> // size_t
#include <stdexcept> // out_of_range
#include <concepts>
#include <ranges>
#include <numeric> // reduce
#include <functional> // multiplies

#include "../../debug/constexpr_assert.hpp"
#include "../../types/range_constraints.hpp"
#include "../zip.hpp" // czip"

namespace fgl {

/// Used to specify the major extent @ingroup group-utility-lma_utilities
enum class major : bool
{
	column = false,
	row = true
};

namespace linear_matrix_utilities {

/**
@file

@example example/fgl/utility/linear_matrix_alias/utilities.cpp
	An example for @ref group-utility-lma_utilities

@defgroup group-utility-lma_utilities Linear Matrix Alias (utilities)

@brief Used to alias a linear contiguous range as a multi-dimensional matrix.

@see the example program
	@ref example/fgl/utility/linear_matrix_alias/utilities.cpp
@{
*/

/**
@brief Asserts that the ranges are of equal size, and then returns that size.
@details If T_dimensions is @c 0 , asserts that the sizes of @p a and @p b are
	the same and returns the size. Otherwise, asserts the sizes of @p a and
	@p b are equal to  @p T_dimensions , and then returns @p T_dimensions .
@tparam T_dimensions The compile-time number of dimensions. If this is @c 0 ,
	only the sizes of @p a and @p b are compared. Otherwise, the sizes of
	@p a and @p b are compared to @p T_dimensions .
@returns The size of @p a and @p b .
*/
template <std::size_t T_dimensions = 0>
[[nodiscard]] constexpr inline std::size_t assert_dimensions(
	const std::ranges::sized_range auto& a,
	const std::ranges::sized_range auto& b)
noexcept
{
	// this could be a more generic `size_t assert_same_length(T_ranges ...)`
	// probably same header as aligned cast

	if constexpr (T_dimensions == 0)
	{
		const std::size_t size{ std::ranges::size(a) };
		FGL_DEBUG_CONSTEXPR_ASSERT(size == std::ranges::size(b));
		return size;
	}
	else
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(std::ranges::size(a) == T_dimensions);
		FGL_DEBUG_CONSTEXPR_ASSERT(std::ranges::size(b) == T_dimensions);
		return T_dimensions;
	}
}

/**
@brief Convert dimensional indexes using offsets to produce a @p T_out
	linear index.
@tparam T_dimensions The compile-time number of dimensions. Must be either
	equal the size of both ranges, or @c 0 in which case the number of
	dimensions will be dynamically determined by the sizes of the ranges.
@param indexes The multi-dimensional indexes to be converted
@param offsets The multi-dimensional offsets used for conversion
@returns The linear index corresponding to the multi-dimensional @p indexes
	for the given @p offsets .
*/
template <std::size_t T_dimensions = 0, std::integral T_out = std::size_t>
[[nodiscard]] constexpr inline T_out matrix_to_linear(
	const fgl::contiguous_range_convertible_to<T_out> auto& indexes,
	const fgl::contiguous_range_convertible_to<T_out> auto& offsets)
noexcept
{
	const std::size_t dimensions{
		assert_dimensions<T_dimensions>(indexes, offsets)
	};
	T_out index{ 0 };
	for (const auto& [i, o] : fgl::czip(dimensions, indexes, offsets))
		index += static_cast<T_out>(i) * static_cast<T_out>(o);
	return index;
}

/**
@brief Calculates and assigns dimensional offsets for the @p T_major of
	@p bounds .
@details
@tparam T_dimensions The compile-time number of dimensions. Must be either
	equal the size of both ranges, or @c 0 in which case the number of
	dimensions will be dynamically determined by the sizes of the ranges.
@tparam T_major The major extent to calculate offsets for.
@tparam T_bounds The integral range type of the multi-dimensional bounds.
@tparam T_offsets The output range type which is assignable from the
	underlying type of @p T_bounds .
@param bounds The multi-dimensional bounds of the aliased range.
@param[out] out_offsets A range to which the calculated offsets will be
	assigned. The underlying type of the range must be assignable from the
	underlying type of the bounds range. In other words, if the bounds type is
	@c T, the offsets type must be assignable from @c T . For concept
	details, refer to @ref group-types-range_constraints.
*/
template
<
	std::size_t T_dimensions = 0,
	major T_major = major::row,
	fgl::random_access_range_integral T_bounds,
	fgl::random_access_range_assignable_from_range<T_bounds> T_offsets
>
constexpr inline void calculate_offsets(
	const T_bounds& bounds,
	T_offsets& out_offsets)
noexcept
{
	const std::size_t dimensions{
		assert_dimensions<T_dimensions>(bounds, out_offsets)
	};
	using out_t = std::ranges::range_value_t<T_bounds>;
	if constexpr (T_major == major::row)
	{
		for (std::size_t i{ 1 }; i < dimensions; ++i)
		{
			out_offsets[i-1] = std::reduce(
				&bounds[i],
				&bounds[dimensions],
				out_t{ 1 },
				std::multiplies<out_t>()
			);
		}
		out_offsets[dimensions-1] = 1;
	}
	else // if (T_major == major::column)
	{
		out_offsets[0] = 1;
		for (std::size_t i{ 1 }; i < dimensions; ++i)
		{
			out_offsets[i] = std::reduce(
				&bounds[0],
				&bounds[i],
				out_t{ 1 },
				std::multiplies<out_t>()
			);
		}
	}
}

/**
@brief A helper wrapper for <tt>@ref calculate_offsets()</tt> which constructs,
	populates, and then returns a @c std::array
@tparam T_dimensions The compile-time number of dimensions. Must be either
	equal the size of the bounds range, or @c 0 in which case the number of
	dimensions will be dynamically determined by the size of the range.
@tparam T_major The major extent to calculate offsets for.
@tparam T_bounds The integral range type of the multi-dimensional bounds.
@param bounds The multi-dimensional bounds of the aliased range.
@returns A @c std::array populated with the calculated offsets; its type is
	the cv-removed value type of @p bounds .
*/
template
<
	std::size_t T_dimensions,
	major T_major = major::row,
	fgl::random_access_range_integral T_bounds
>
[[nodiscard]] constexpr inline auto make_offsets(const T_bounds& bounds)
noexcept
{
	using out_type = std::remove_cv_t<std::ranges::range_value_t<T_bounds>>;
	std::array<out_type, T_dimensions> array_out;
	calculate_offsets<T_dimensions, T_major>(bounds, array_out);
	return array_out;
}

/**
@brief Determines whether or not any multi-dimensional is out-of-bounds.
@returns @c true if an index is greater than or equal to its corresponding
	bound.
*/
template <std::size_t T_dimensions = 0>
constexpr inline bool out_of_bounds(
	const fgl::random_access_range_integral auto& indexes,
	const fgl::random_access_range_integral auto& bounds)
noexcept
{
	const std::size_t dimensions{
		assert_dimensions<T_dimensions>(indexes, bounds)
	};
	for(const auto& [index, bound] : fgl::czip(dimensions, indexes, bounds))
	{
		if (index >= bound)
			return true;
	}
	return false;
}

/**
@brief A throwing wrapper for <tt>@ref out_of_bounds()</tt>
@throws std::out_of_range if <tt>@ref out_of_bounds()</tt> returns @c true
*/
template <std::size_t T_dimensions = 0>
constexpr inline void check_index_bounds(
	const fgl::random_access_range_integral auto& indexes,
	const fgl::random_access_range_integral auto& bounds)
{
	if (out_of_bounds<T_dimensions>(indexes, bounds))
		throw std::out_of_range("dimensional indexes exceed bounds.");
}

} // namespace linear_matrix_utilities

///@}
} // namespace fgl

#endif // FGL_TYPES_DIMENSIONAL_ALIAS_UTILITIES_HPP_INCLUDED

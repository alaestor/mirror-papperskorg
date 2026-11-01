#pragma once
#ifndef FGL_UTILITY_LINEAR_MATRIX_ALIAS_STATIC_HPP_INCLUDED
#define FGL_UTILITY_LINEAR_MATRIX_ALIAS_STATIC_HPP_INCLUDED
#include "../../environment/libfgl_compatibility_check.hpp"

#include <cstddef> // size_t
#include <stdexcept> // invalid_argument, out_of_range
#include <iterator> // contiguous_iterator
#include <concepts>
#include <ranges>

#include "./utilities.hpp"

namespace fgl {

/**
@file

@example example/fgl/utility/linear_matrix_alias/static.cpp
	An example for @ref group-utility-lma_static

@defgroup group-utility-lma_static Linear Matrix Alias (static)

@brief A linear matrix alias which prioritizes static optimizations

@see the example program
	@ref example/fgl/utility/linear_matrix_alias/static.cpp
@{
*/

/**
@brief A static container for linear matrix alias information, such as
	dimensions, bounds, and offsets.
@remarks This class is used by internally by
	<tt>@ref static_linear_matrix_alias</tt>. Using the static alias doesn't
	require interacting with this type directly. However, It remains part of
	the public interface as it may be useful for implementing other
	@ref group-utility-lma_utilities abstractions.
@tparam T_major The major order of the matrix
@tparam T_bounds The multi-dimensional bounds of the matrix
*/
template <major T_major, std::size_t ... T_bounds>
struct static_linear_matrix_info
{
	static constexpr std::size_t dimensions{ sizeof...(T_bounds) };
	using array_t = std::array<std::size_t, dimensions>;
	static constexpr array_t bounds{ T_bounds... };
	static constexpr array_t offsets{
		fgl::linear_matrix_utilities::make_offsets<dimensions, T_major>(bounds)
	};

	/**
	@brief A wrapper which calls
		<tt>@ref fgl::linear_matrix_utilities::out_of_bounds()</tt> and
		passes the static dimensions and bounds members.
	*/
	[[nodiscard]] static constexpr bool out_of_bounds(const array_t& indexes)
	{
		return fgl::linear_matrix_utilities::out_of_bounds
			<dimensions>(indexes, bounds);
	}

	/**
	@brief A wrapper which calls
		<tt>@ref fgl::linear_matrix_utilities::check_index_bounds()</tt> and
		passes the static dimensions and bounds members.
	*/
	static constexpr void check_bounds(const array_t& indexes)
	{
		fgl::linear_matrix_utilities::check_index_bounds
			<dimensions>(indexes, bounds);
	}

	/**
	@brief A wrapper which calls
		<tt>@ref fgl::linear_matrix_utilities::matrix_to_linear()</tt> and
		passes the static dimensions and offsets members.
	*/
	template <std::integral T_out = std::ptrdiff_t>
	[[nodiscard]] static constexpr
	T_out convert_indexes(const array_t& indexes) noexcept
	{
		return fgl::linear_matrix_utilities::matrix_to_linear
			<dimensions, T_out>(indexes, offsets);
	}
};

/**
@brief A linear matrix alias which prioritizes static optimizations
@remarks Having static major, dimensions, and bounds makes certain compile-time
	optimizations possible and more likely to be used. For example: loop
	unrolling, @c constexpr calculation of offsets, reduced data duplication,
	and shared <tt>@ref static_linear_matrix_info</tt> memory footprint
	between static aliases of the same major and bounds.
@tparam T_iter The underlying linear contiguous iterator type
@tparam T_major The major order of the matrix
@tparam T_bounds The multi-dimensional bounds of the matrix
*/
template
<
	std::contiguous_iterator T_iter,
	major T_major,
	std::size_t ... T_bounds
>
class static_linear_matrix_alias
{
	T_iter m_iter;

	using static_info = fgl::static_linear_matrix_info<T_major, T_bounds...>;

	static constexpr void check_range_bounds(
		const std::ranges::contiguous_range auto& range)
	{
		constexpr std::size_t alias_len{ // TODO C++23 make static
			std::reduce(
				static_info::bounds.cbegin(),
				static_info::bounds.cend(),
				std::size_t{ 1 },
				std::multiplies<std::size_t>()
			)
		};
		if (alias_len != std::ranges::size(range)) throw std::invalid_argument(
			"static_linear_matrix_alias() range size must equal"
			" the product of the dimensional bounds"
		);
	}

public:
	using array_t = static_info::array_t;
	using difference_type = std::iter_difference_t<T_iter>;

	///@{ @name Constructors

	/**
	@brief Constructs a static linear matrix alias from an iterator
	@param iter The underlying iterator to the beginning of the linear
		contiguous range to alias
	*/
	[[nodiscard]] explicit constexpr
	static_linear_matrix_alias(T_iter iter) noexcept
	: m_iter(iter)
	{}

	/**
	@brief Constructs a static linear matrix alias from a range
	@param range The contiguous range from which the underlying iterator is
		taken. The product of the multi-dimensional bounds must equal the
		size of the range.
	@throws std::invalid_argument if the range size does not equal the product
		of the multi-dimensional bounds
	*/
	[[nodiscard]] explicit constexpr
	static_linear_matrix_alias(std::ranges::contiguous_range auto& range)
	: static_linear_matrix_alias(std::ranges::begin(range))
	{ check_range_bounds(range); }
	///@}

	///@returns The static number of dimensions
	[[nodiscard]] static constexpr std::size_t dimensions() noexcept
	{ return static_info::dimensions; }

	///@returns A reference to the static array of multi-dimensional bounds.
	[[nodiscard]] static constexpr const array_t& bounds() noexcept
	{ return static_info::bounds; }

	///@returns A reference to the static array of multi-dimensional offsets.
	[[nodiscard]] static constexpr const array_t& offsets() noexcept
	{ return static_info::offsets; }

	///@returns The static major order of the matrix
	[[nodiscard]] static constexpr major get_major() noexcept
	{ return T_major; }

	///@returns @c true if the major order is @c row , @c false otherwise
	[[nodiscard]] static constexpr bool is_row_major() noexcept
	{ return T_major == major::row; }

	///@returns @c true if the major order is @c column , @c false otherwise
	[[nodiscard]] static constexpr bool is_column_major() noexcept
	{ return T_major == major::column; }

	/// @returns The underlying linear iterator
	[[nodiscard]] constexpr auto iterator() const
	{ return m_iter; }

	/**
	@brief Changes the underlying linear iterator. Not bounds-checked.
	@param iter The new underlying iterator. Not bounds-checked.
	*/
	constexpr void set_iterator(T_iter iter) noexcept
	{ m_iter = iter; }

	/**
	@brief Changes the underlying linear iterator.
	@param range The contiguous range from which the underlying iterator is
		taken. The product of the multi-dimensional bounds must equal the
		size of the range.
	@throws std::invalid_argument if the range size does not equal the product
		of the multi-dimensional bounds
	*/
	template <std::ranges::contiguous_range T_range>
	requires std::same_as<std::ranges::iterator_t<T_range>, T_iter>
	constexpr void set_iterator(T_range& range)
	{
		check_range_bounds(range);
		set_iterator(std::ranges::begin(range));
	}

	/**
	@brief Converts multi-dimensional alias indexes to a linear index.
	@param indexes The multi-dimensional indexes to convert.
	@returns The linear index.
	*/
	[[nodiscard]] static constexpr
	difference_type convert_indexes(const array_t& indexes) noexcept
	{ return static_info::template convert_indexes<difference_type>(indexes); }

	///@{ @name Element Access
	/**
	@brief Accesses the element corresponding to a multi-dimensional index.
	@warning The indexes are bounds-checked by an assertion; this won't be
		bounds-checked if @c NDEBUG is defined.
	@param indexes The multi-dimensional indexes of the element to access.
	@returns A reference to the element.
	*/
	[[nodiscard]] constexpr auto& operator[](const array_t& indexes)
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(not static_info::out_of_bounds(indexes));
		return m_iter[static_info::convert_indexes(indexes)];
	}

	/**
	@brief Accesses the element corresponding to a multi-dimensional index.
	@warning The indexes are bounds-checked by an assertion; this won't be
		bounds-checked if @c NDEBUG is defined.
	@param indexes The multi-dimensional indexes of the element to access.
	@returns A <tt>const</tt>-qualified reference to the element.
	*/
	[[nodiscard]] constexpr
	const auto& operator[](const array_t& indexes) const
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(not static_info::out_of_bounds(indexes));
		return m_iter[static_info::convert_indexes(indexes)];
	}

	/**
	@brief Safely accesses the element corresponding to a multi-dimensional
		index.
	@param indexes The multi-dimensional indexes of the element to access.
	@returns A reference to the element.
	@throws std::out_of_range If an index is out of bounds
	*/
	[[nodiscard]] constexpr auto& at(const array_t& indexes)
	{
		static_info::check_bounds(indexes);
		return operator[](indexes);
	}

	/**
	@brief Safely accesses the element corresponding to a multi-dimensional
		index.
	@param indexes The multi-dimensional indexes of the element to access.
	@returns A <tt>const</tt>-qualified reference to the element.
	@throws std::out_of_range If an index is out of bounds
	*/
	[[nodiscard]] constexpr const auto& at(const array_t& indexes) const
	{
		static_info::check_bounds(indexes);
		return operator[](indexes);
	}
	///@}
};

/**
@{ @name Static Alias Helper factories
@details @parblock
	The following functions are helper factories for creating static linear
	matrix aliases. They exist to simplify the noisy template boilerplate
	that would normally be required to construct an alias.

	The simplified syntax:
	@code
	auto alias{ make_matrix_alias<x,y,z>(iter) };
	auto alias{ make_matrix_alias<major::column, x,y,z>(iter) };
	auto alias{ make_matrix_alias<x,y,z>(range) };
	auto alias{ make_matrix_alias<major::column, x,y,z>(range) };
	@endcode

@endparblock

@tparam T_major The major order of the matrix.
@tparam T_bounds The <tt>size_t</tt>-convertible multi-dimensional bounds of
	the matrix.
@param iter The contiguous iterator which is used to construct the alias.
@param range The contiguous range which is used to construct the
	alias. (bounds-checked)
*/

/// Constructs a row major static matrix alias from an iterator
template <std::convertible_to<std::size_t> auto ... T_bounds>
[[nodiscard]] constexpr inline
auto make_matrix_alias(std::contiguous_iterator auto iter) noexcept
{
	return static_linear_matrix_alias
		<
			decltype(iter),
			major::row, // detault
			static_cast<std::size_t>(T_bounds)...
		>(iter);
}

/// Constructs a static matrix alias from an iterator
template <major T_major, std::convertible_to<std::size_t> auto ... T_bounds>
[[nodiscard]] constexpr inline
auto make_matrix_alias(std::contiguous_iterator auto iter) noexcept
{
	return static_linear_matrix_alias
		<
			decltype(iter),
			T_major,
			static_cast<std::size_t>(T_bounds)...
		>(iter);
}

/// Constructs a row major static matrix alias from a bounds-checked range
template <std::convertible_to<std::size_t> auto ... T_bounds>
[[nodiscard]] constexpr inline
auto make_matrix_alias(std::ranges::contiguous_range auto& range)
{
	return static_linear_matrix_alias
		<
			std::ranges::iterator_t<decltype(range)>,
			major::row, // default
			static_cast<std::size_t>(T_bounds)...
		>(range);
}

/// Constructs a static matrix alias from a bounds-checked range
template <major T_major, std::convertible_to<std::size_t> auto ... T_bounds>
[[nodiscard]] constexpr
auto make_matrix_alias(std::ranges::contiguous_range auto& range)
{
	return static_linear_matrix_alias
		<
			std::ranges::iterator_t<decltype(range)>,
			T_major,
			static_cast<std::size_t>(T_bounds)...
		>(range);
}

///@}

///@}
}// namespace fgl

#endif // FGL_UTILITY_LINEAR_MATRIX_ALIAS_STATIC_HPP_INCLUDED

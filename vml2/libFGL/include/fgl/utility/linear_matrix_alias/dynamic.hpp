#pragma once
#ifndef FGL_UTILITY_LINEAR_MATRIX_ALIAS_DYNAMIC_HPP_INCLUDED
#define FGL_UTILITY_LINEAR_MATRIX_ALIAS_DYNAMIC_HPP_INCLUDED
#include "../../environment/libfgl_compatibility_check.hpp"

#include <cstddef> // size_t
#include <stdexcept> // invalid_argument, out_of_range
#include <iterator> // contiguous_iterator iter_difference_t
#include <concepts>
#include <ranges>

#include "../../types/range_constraints.hpp"
#include "./utilities.hpp"

namespace fgl {

/**
@file

@example example/fgl/utility/linear_matrix_alias/dynamic.cpp
	An example for @ref group-utility-lma_dynamic

@defgroup group-utility-lma_dynamic Linear Matrix Alias (dynamic)

@brief A linear matrix alias which prioritizes run-time flexibility.

@see the example program
	@ref example/fgl/utility/linear_matrix_alias/dynamic.cpp
@{
*/

/**
@brief A linear matrix alias which prioritizes run-time flexibility.
@details Unlike the <tt>@ref static_linear_matrix_alias</tt>, the bounds and
	major order of the dynamic alias can be changed at run-time. However the
	number of dimensions must still be known at compile-time.
@tparam T_iter The underlying linear contiguous iterator type
@tparam T_dimensions The number of dimensions
*/
template <std::contiguous_iterator T_iter, std::size_t T_dimensions>
class dynamic_linear_matrix_alias
{
public:
	using array_t = std::array<std::size_t, T_dimensions>;
	using difference_type = std::iter_difference_t<T_iter>;

	///@{ @name Constructors

	dynamic_linear_matrix_alias() = delete;
	dynamic_linear_matrix_alias(const dynamic_linear_matrix_alias&) = delete;
	dynamic_linear_matrix_alias(dynamic_linear_matrix_alias&&) = delete;
	void operator=(const dynamic_linear_matrix_alias&) = delete;

	/**
	@brief Constructs a dynamic linear matrix alias from an iterator
	@param iter The underlying iterator to the beginning of the linear
		contiguous range to alias
	*/
	[[nodiscard]] explicit constexpr dynamic_linear_matrix_alias(
		T_iter iter,
		const array_t& bounds,
		const major major = major::row)
	noexcept
	:
		m_major(major),
		m_bounds(bounds),
		m_offsets{
			m_major == major::row
				? fgl::linear_matrix_utilities::make_offsets
					<T_dimensions, major::row>(m_bounds)
				: fgl::linear_matrix_utilities::make_offsets
					<T_dimensions, major::column>(m_bounds)
		},
		m_iter(iter)
	{}

	/**
	@brief Constructs a dynamic linear matrix alias from a range
	@param range The contiguous range from which the underlying iterator is
		taken. The product of the multi-dimensional bounds must equal the
		size of the range.
	@throws std::invalid_argument if the range size does not equal the product
		of the multi-dimensional bounds
	*/
	[[nodiscard]] explicit constexpr dynamic_linear_matrix_alias(
		std::ranges::contiguous_range auto& range,
		const array_t& bounds,
		const major major = major::row)
	: dynamic_linear_matrix_alias(std::ranges::begin(range), bounds, major)
	{ check_range_bounds(range, bounds); }
	///@}

	///@returns The static number of dimensions
	[[nodiscard]] constexpr std::size_t dimensions() const noexcept
	{ return T_dimensions; }

	///@returns A @c const reference to the array of multi-dimensional bounds.
	[[nodiscard]] constexpr const array_t& bounds() const noexcept
	{ return m_bounds; }

	/// Changes the bounds of the matrix (not bounds-checked)
	constexpr void set_bounds(const array_t& bounds) noexcept
	{
		m_bounds = bounds;
		recalculate_offsets();
	}

	///@returns A @c const reference to the array of multi-dimensional offsets.
	[[nodiscard]] constexpr const array_t& offsets() const noexcept
	{ return m_offsets; }

	///@returns The major order of the matrix
	[[nodiscard]] constexpr major get_major() const noexcept
	{ return m_major; }

	///@returns @c true if the major order is @c row , @c false otherwise
	[[nodiscard]] constexpr bool is_row_major() const noexcept
	{ return get_major() == fgl::major::row; }

	///@returns @c true if the major order is @c column , @c false otherwise
	[[nodiscard]] constexpr bool is_column_major() const noexcept
	{ return get_major() == fgl::major::column; }

	/// Toggles between major orders
	constexpr void switch_major() noexcept
	{
		m_major = is_row_major() ? major::column : major::row;
		recalculate_offsets();
	}

	/// Sets the major order to @p new_major
	constexpr void set_major(const fgl::major new_major) noexcept
	{
		if (m_major != new_major)
		{
			m_major = new_major;
			recalculate_offsets();
		}
	}

	/// @returns The underlying linear iterator
	[[nodiscard]] constexpr auto iterator() const noexcept
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
		check_range_bounds(range, m_bounds);
		set_iterator(std::ranges::begin(range));
	}

	/**
	@brief Changes the underlying linear iterator and matrix bounds.
	@param iter The new underlying iterator. Not bounds-checked.
	@param bounds The new multi-dimensional bounds of the matrix.
	*/
	constexpr void update(T_iter iter, const array_t& bounds) noexcept
	{
		set_iterator(iter);
		set_bounds(bounds);
	}

	/**
	@brief Changes the underlying linear iterator, matrix bounds, and major.
	@param iter The new underlying iterator. Not bounds-checked.
	@param bounds The new multi-dimensional bounds of the matrix.
	@param new_major The new major order of the matrix.
	*/
	constexpr void update(
		T_iter iter,
		const array_t& bounds,
		const fgl::major new_major) noexcept
	{
		set_major(new_major);
		update(iter, bounds);
	}

	/**
	@brief Changes the underlying linear iterator and matrix bounds.
	@param range The contiguous range from which the underlying iterator is
		taken. The product of the multi-dimensional @p bounds must equal the
		size of the range.
	@param bounds The new multi-dimensional bounds of the matrix.
	@throws std::invalid_argument if the range size does not equal the product
		of the @p bounds
	*/
	template <std::ranges::contiguous_range T_range>
	requires std::same_as<std::ranges::iterator_t<T_range>, T_iter>
	constexpr void update(T_range& range, const array_t& bounds)
	{
		check_range_bounds(range, bounds);
		update(std::ranges::begin(range), bounds);
	}

	/**
	@brief Changes the underlying linear iterator, matrix bounds, and major.
	@param range The contiguous range from which the underlying iterator is
		taken. The product of the multi-dimensional @p bounds must equal the
		size of the range.
	@param bounds The new multi-dimensional bounds of the matrix.
	@param new_major The new major order of the matrix.
	@throws std::invalid_argument if the range size does not equal the product
		of the @p bounds
	*/
	template <std::ranges::contiguous_range T_range>
	requires std::same_as<std::ranges::iterator_t<T_range>, T_iter>
	constexpr void update(
		T_range& range,
		const array_t& bounds,
		const fgl::major new_major)
	{
		check_range_bounds(range, bounds);
		set_major(new_major);
		update(std::ranges::begin(range), bounds);
	}

	/**
	@brief Converts multi-dimensional alias indexes to a linear index.
	@param indexes The multi-dimensional indexes to convert.
	@returns The linear index.
	*/
	[[nodiscard]] constexpr
	difference_type convert_indexes(const array_t& indexes) const noexcept
	{
		using fgl::linear_matrix_utilities::matrix_to_linear;
		return matrix_to_linear<T_dimensions, difference_type>(
			indexes, m_offsets
		);
	}

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
		FGL_DEBUG_CONSTEXPR_ASSERT(
			not fgl::linear_matrix_utilities::out_of_bounds(
				indexes,
				m_bounds
			)
		);
		return m_iter[convert_indexes(indexes)];
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
		FGL_DEBUG_CONSTEXPR_ASSERT(
			not fgl::linear_matrix_utilities::out_of_bounds(
				indexes,
				m_bounds
			)
		);
		return m_iter[convert_indexes(indexes)];
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
		fgl::linear_matrix_utilities::check_index_bounds
			<T_dimensions>(indexes, m_bounds);
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
		fgl::linear_matrix_utilities::check_index_bounds
			<T_dimensions>(indexes, m_bounds);
		return operator[](indexes);
	}
	///@}

private:

	/// Recalculates @ref m_offsets based on @ref m_bounds and @ref m_major .
	constexpr void recalculate_offsets() noexcept
	{
		if (is_row_major())
		{
			fgl::linear_matrix_utilities::calculate_offsets
				<T_dimensions, major::row>(m_bounds, m_offsets);
		}
		else
		{
			fgl::linear_matrix_utilities::calculate_offsets
				<T_dimensions, major::column>(m_bounds, m_offsets);
		}
	}

	/// Checks that the @p range size equals the product of the @p bounds .
	static constexpr void check_range_bounds(
		const std::ranges::contiguous_range auto& range,
		const array_t& bounds)
	{
		const std::size_t alias_len{
			std::reduce(
				bounds.cbegin(),
				bounds.cend(),
				std::size_t{ 1 },
				std::multiplies<std::size_t>()
			)
		};
		if (alias_len != std::ranges::size(range)) throw std::invalid_argument(
			"dynamic_linear_matrix_alias() range size must equal"
			" the product of the dimensional bounds"
		);
	}

	fgl::major m_major;
	array_t m_bounds;
	array_t m_offsets;
	T_iter m_iter;
};

/// Deduction guide
template <std::ranges::contiguous_range T_range, std::size_t T_dimensions>
dynamic_linear_matrix_alias(
	T_range&,
	const std::array<std::size_t, T_dimensions>&,
	const fgl::major
)->dynamic_linear_matrix_alias<std::ranges::iterator_t<T_range>, T_dimensions>;


/**
@{ @name Dynamic Alias Helper factories
@details @parblock
	The following functions are helper factories for creating dynamic linear
	matrix aliases. They exist to simplify the noisy casts and @c std::array
	temporary that would often be used when construct a dynamic alias.

	The simplified syntax:
	@code
	auto alias{ make_matrix_alias(iter, x,y,z) };
	auto alias{ make_matrix_alias(iter, major::column, x,y,z) };
	auto alias{ make_matrix_alias(range, x,y,z) };
	auto alias{ make_matrix_alias(range, major::column, x,y,z) };
	@endcode

@endparblock

@param iter The contiguous iterator which is used to construct the alias.
@param range The contiguous range which is used to construct the
	alias. (bounds-checked)
@param major The major order of the matrix.
@param bounds The multi-dimensional bounds of the matrix.
*/

/// Constructs a row major dynamic matrix alias from an iterator
[[nodiscard]] constexpr inline auto make_matrix_alias(
	std::contiguous_iterator auto iter,
	const std::convertible_to<std::size_t> auto ... bounds)
noexcept
{
	return dynamic_linear_matrix_alias(
		iter, std::array{ static_cast<std::size_t>(bounds)... }, true // row
	);
}

/// Constructs a dynamic matrix alias from an iterator
[[nodiscard]] constexpr inline auto make_matrix_alias(
	std::contiguous_iterator auto iter,
	const major major,
	const std::convertible_to<std::size_t> auto ... bounds)
noexcept
{
	return dynamic_linear_matrix_alias(
		iter, std::array{ static_cast<std::size_t>(bounds)... }, major
	);
}

/// Constructs a row major dynamic matrix alias from a bounds-checked range
[[nodiscard]] constexpr inline auto make_matrix_alias(
	std::ranges::contiguous_range auto& range,
	const std::convertible_to<std::size_t> auto ... bounds)
{
	return dynamic_linear_matrix_alias(
		range, std::array{ static_cast<std::size_t>(bounds)... }, true // row
	);
}

/// Constructs a dynamic matrix alias from a bounds-checked range
[[nodiscard]] constexpr inline auto make_matrix_alias(
	std::ranges::contiguous_range auto& range,
	const major major,
	const std::convertible_to<std::size_t> auto ... bounds)
{
	return dynamic_linear_matrix_alias(
		range, std::array{ static_cast<std::size_t>(bounds)... }, major
	);
}

///@}

///@}
}// namespace fgl

#endif // FGL_UTILITY_LINEAR_MATRIX_ALIAS_DYNAMIC_HPP_INCLUDED

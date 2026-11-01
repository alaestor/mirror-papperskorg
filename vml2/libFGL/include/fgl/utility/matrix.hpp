#pragma once
#ifndef FGL_UTILITY_MATRIX_HPP_INCLUDED
#define FGL_UTILITY_MATRIX_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cstddef> // size_t
#include <array>

#include "./linear_matrix_alias/utilities.hpp"
#include "./linear_matrix_alias/static.hpp" //static_linear_matrix_info

namespace fgl {

/**
@file

@example example/fgl/utility/matrix.cpp
	An example for @ref group-utility-matrix

@defgroup group-utility-matrix Matrix

@brief A multi-dimensional, row-major array

@details Combines the behavior of a <tt>@ref static_linear_matrix_alias</tt>
	with a @c std::array .

@see the example program @ref example/fgl/utility/matrix.cpp
@{
*/

/**
@copydoc group-utility-matrix
@note The range and its iterators are linear; multi-dimensional indexing is
	only possible thru the container.
@tparam T The matrix element type.
@tparam T_dimensions The number of dimensions
*/
template <typename T, std::size_t ... T_bounds>
class matrix : public std::array<T, (T_bounds * ...)>
{
	using static_info = fgl::static_linear_matrix_info
		<fgl::major::row, T_bounds...>;

	public:
	/// A @c std::array of @c std::size_t dimension correlated by index.
	using dimensional_t = static_info::array_t;

	/// The inhereted @c std::array type.
	using base_t = std::array<T, (T_bounds * ...)>;

	///@returns The static number of dimensions
	[[nodiscard]] static constexpr std::size_t dimensions() noexcept
	{ return static_info::dimensions; }

	///@returns A reference to the static array of multi-dimensional bounds.
	[[nodiscard]] static constexpr const dimensional_t& bounds() noexcept
	{ return static_info::bounds; }

	///@returns A reference to the static array of multi-dimensional offsets.
	[[nodiscard]] static constexpr const dimensional_t& offsets() noexcept
	{ return static_info::offsets; }

	/**
	@brief Converts multi-dimensional indexes to a linear index.
	@param indexes The multi-dimensional indexes to convert.
	@returns The linear index.
	*/
	[[nodiscard]] static constexpr
	std::size_t convert_indexes(const dimensional_t& indexes) noexcept
	{ return static_cast<std::size_t>(static_info::convert_indexes(indexes)); }

	/// Self-downcasting wrapper
	[[nodiscard]] constexpr base_t& linear_array() noexcept
	{ return *static_cast<base_t*>(this); }

	/// Self-downcasting wrapper
	[[nodiscard]] constexpr const base_t& linear_array() const noexcept
	{ return *static_cast<const base_t*>(this); }

	constexpr matrix& operator=(const matrix&) = default;
	constexpr matrix& operator=(const base_t& rhs)
	{
		linear_array() = rhs;
		return *this;
	}

	///@{ @name Element Access

	constexpr auto operator[](base_t::size_type) = delete;
	constexpr auto operator[](base_t::size_type) const = delete;
	constexpr auto at(base_t::size_type) = delete;
	constexpr auto at(base_t::size_type) const = delete;

	/// Indexes the base linear array via @c std::array::operator[]()
	[[nodiscard]] constexpr auto& linear_index(const std::size_t index)
	{ return linear_array()[index]; }

	/// Indexes the base linear array via @c std::array::operator[]()
	[[nodiscard]] constexpr
	const auto& linear_index(const std::size_t index) const
	{ return linear_array()[index]; }

	/// Indexes the base linear array via @c std::array::at()
	[[nodiscard]] constexpr auto& linear_at(const std::size_t index)
	{ return linear_array().at(index); }

	/// Indexes the base linear array via @c std::array::at()
	[[nodiscard]] constexpr
	const auto& linear_at(const std::size_t index) const
	{ return linear_array().at(index); }

	/**
	@brief Accesses the element corresponding to a multi-dimensional index.
	@warning The indexes are bounds-checked by an assertion; this won't be
		bounds-checked if @c NDEBUG is defined.
	@param indexes The multi-dimensional indexes of the element to access.
	@returns A reference to the element.
	*/
	[[nodiscard]] constexpr auto& operator[](const dimensional_t& indexes)
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(not static_info::out_of_bounds(indexes));
		return linear_array()[convert_indexes(indexes)];
	}

	/**
	@brief Accesses the element corresponding to a multi-dimensional index.
	@warning The indexes are bounds-checked by an assertion; this won't be
		bounds-checked if @c NDEBUG is defined.
	@param indexes The multi-dimensional indexes of the element to access.
	@returns A <tt>const</tt>-qualified reference to the element.
	*/
	[[nodiscard]] constexpr
	const auto& operator[](const dimensional_t& indexes) const
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(not static_info::out_of_bounds(indexes));
		return linear_array()[convert_indexes(indexes)];
	}

	/**
	@brief Safely accesses the element corresponding to a multi-dimensional
		index.
	@param indexes The multi-dimensional indexes of the element to access.
	@returns A reference to the element.
	@throws std::out_of_range If an index is out of bounds
	*/
	[[nodiscard]] constexpr auto& at(const dimensional_t& indexes)
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
	[[nodiscard]] constexpr const auto& at(const dimensional_t& indexes) const
	{
		static_info::check_bounds(indexes);
		return operator[](indexes);
	}

	///@}

	/* for a T_major template version:
	[[nodiscard]] constexpr major major() const noexcept
	{ return T_major; }
	[[nodiscard]] constexpr bool is_row_major() const noexcept
	{ return T_major == major::row; }
	[[nodiscard]] constexpr bool is_column_major() const noexcept
	{ return T_major == major::column; }
	[[nodiscard]] constexpr reference back()
	{ return operator[]({T_bounds-1 ...}); }
	[[nodiscard]] constexpr const_reference back() const
	{ return operator[]({T_bounds-1 ...}); }
	need to override begin and end iterators
	*/
};

/*
template <typename T, std::size_t ... T_bounds>
[[nodiscard]] constexpr bool operator==(
	const fgl::matrix<T, T_bounds...>& lhs,
	const fgl::matrix<T, T_bounds...>& rhs)
{ return lhs.linear_array() == rhs.linear_array(); }

template <typename T, std::size_t ... T_bounds>
[[nodiscard]] constexpr auto operator<=>(
	const fgl::matrix<T, T_bounds...>& lhs,
	const fgl::matrix<T, T_bounds...>& rhs)
{ return lhs.linear_array() <=> rhs.linear_array(); }
//*/


/**
@{ @name Comparison operators
@relates matrix
*/

template <typename T, std::size_t ... T_bounds>
[[nodiscard]] constexpr bool operator==(
	const std::array<T, (T_bounds * ...)>& lhs,
	const fgl::matrix<T, T_bounds...>& rhs)
{ return lhs == rhs.linear_array(); }

template <typename T, std::size_t ... T_bounds>
[[nodiscard]] constexpr auto operator<=>(
	const std::array<T, (T_bounds * ...)>& lhs,
	const fgl::matrix<T, T_bounds...>& rhs)
{ return lhs <=> rhs.linear_array(); }
///@}

///@}
} // namespace fgl

#endif // FGL_UTILITY_MATRIX_HPP_INCLUDED

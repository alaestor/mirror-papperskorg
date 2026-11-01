#pragma once
#ifndef FGL_UTILITY_ENUMERATE_HPP_INCLUDED
#define FGL_UTILITY_ENUMERATE_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cstddef> // size_t
#include <concepts> // integral
#include <limits> // numeric_limits
#include <ranges> // forward_range

#include "../utility/zip.hpp"

namespace fgl {

/**
@file

@example example/fgl/utility/enumerate.cpp
	An example for @ref group-utility-enumerate

@defgroup group-utility-enumerate Enumerate

@brief An enumerated (counting) zipped range.

@details An enumerated zip range is a zip range whose first element is a
	forward iterator for an incrementing integer counter. This is also known
	as a "counted" range, starting from @c 0 .

@see the example program @ref example/fgl/utility/enumerate.cpp
@{
*/

///@cond FGL_INTERNAL_DOCS
namespace internal{
///@todo this could be its own public-facing component, random access
template<std::integral T = std::size_t>
struct counting_iterator
{
	using difference_type = std::ptrdiff_t;
	using value_type = T;
	value_type counter{};

	[[nodiscard]] constexpr counting_iterator() noexcept = default;

	[[nodiscard]] constexpr counting_iterator(
		const value_type init) noexcept
	: counter{ init }
	{}

	[[nodiscard]] constexpr value_type size() const noexcept
	{return std::numeric_limits<value_type>::max();}

	constexpr counting_iterator& operator++() noexcept
	{
		++counter;
		return *this;
	}

	[[nodiscard]] constexpr counting_iterator operator++(int) noexcept
	{
		const auto copy{ *this };
		++*this;
	}

	[[nodiscard]] constexpr value_type operator*() const noexcept
	{ return counter; }

	[[nodiscard]] constexpr bool operator==(const value_type rhs)
	const noexcept
	{ return counter == rhs; }

	[[nodiscard]] constexpr auto operator<=>(const value_type rhs)
	const noexcept
	{ return counter <=> rhs; }

	[[nodiscard]] constexpr bool operator==(const counting_iterator& other)
	const noexcept
	{ return counter == other.counter; }

	[[nodiscard]] constexpr auto operator<=>(const counting_iterator& other)
	const noexcept
	{ return counter <=> other.counter; }
};

static_assert(std::forward_iterator<counting_iterator<std::size_t>>);

} // namespace internal
///@endcond

/**
@tparam T The integral type of the counter.
@param ranges The ranges to enumerate.
@returns An enumerated zip range.
@see <tt>@ref fgl::zip()</tt>
*/
template <std::integral T = std::size_t>
constexpr auto enumerate(std::ranges::forward_range auto&& ... ranges)
{
	return std::ranges::subrange(
		internal::forward_zip_iterator(
			internal::counting_iterator<T>{},
			std::begin(std::forward<decltype(ranges)>(ranges))...
		),
		static_cast<zip_sentinel_t>(
			internal::shortest(std::ranges::ssize(ranges)...)
		)
	);
}

/**
@tparam T The integral type of the counter.
@param ranges The ranges to enumerate.
@returns An enumerated zip range.
@see <tt>@ref fgl::czip()</tt>
*/
template <std::integral T = std::size_t>
constexpr auto cenumerate(std::ranges::forward_range auto&& ... ranges)
{
	return std::ranges::subrange(
		internal::forward_zip_iterator(
			internal::counting_iterator<T>{},
			std::cbegin(std::forward<decltype(ranges)>(ranges))...
		),
		static_cast<zip_sentinel_t>(
			internal::shortest(std::ranges::ssize(ranges)...)
		)
	);
}

///@} group-utility-enumerate

}// namespace fgl

#endif // FGL_UTILITY_ENUMERATE_HPP_INCLUDED

#pragma once
#ifndef FGL_UTILITY_ZIP_HPP_INCLUDED
#define FGL_UTILITY_ZIP_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cstddef> // size_t, ptrdiff_t
#include <functional> // ref
#include <iterator>
#include <tuple>
#include <ranges>

#include "../debug/constexpr_assert.hpp"

namespace fgl {

/**
@file

@example example/fgl/utility/zip.cpp
	An example for @ref group-utility-zip

@defgroup group-utility-zip Zip

@brief Iterates over multiple forward ranges of differing types simultaneously

@attention This fascility may be supplanted entirely by C++23's
	<tt>zip_view</tt>, depending its performance. If the standard is
	sufficient or superior, this facility will be removed.

@details @parblock
	A zipped range is a @c std::ranges::subrange composed of an internal
	zip iterator and a size-based <tt>@ref fgl::zip_sentinel_t</tt> end
	sentinel. The length of the zipped range must always be less than or
	greater to the size of the smallest range, otherwise the behavior is
	undefined. The zip iterator is internal because it isn't intended to be
	used directly by the user; usually only by ranged algorithms and
	range-based @c for loops.

	Dreferencing the zip iterator returns a @c std::tuple composed of the
	results from dereferencing the underlying iterators. These may be values
	or reference-wrappers depending on what the underlying iterators return.

	@c czip ranges use <tt>const</tt>-qualified underlying iterators from
	@c std::cbegin , while @c zip ranges use iterators from @c std::begin .
	@endparblock

@see the example program @ref example/fgl/utility/zip.cpp
@{
*/

using zip_sentinel_t = std::ptrdiff_t;

///@cond FGL_INTERNAL_DOCS
namespace internal {

/// iterator used in <tt>@ref fgl::zip</tt> and <tt>@ref fgl::czip</tt> ranges
template <std::forward_iterator ... T_iters>
class forward_zip_iterator final
{
	zip_sentinel_t m_index{ 0 };
	std::tuple<T_iters...> m_iters;

	// maybe an easier way but oh well
	[[nodiscard]] static constexpr auto accessType(auto& v) noexcept
	{ return std::ref(v); }

	[[nodiscard]] static constexpr auto accessType(auto&& v) noexcept
	{ return v; }

	template <std::forward_iterator T>
	using get_iter_ref_t = typename std::iterator_traits<T>::reference;

public:
	using value_type = std::tuple<get_iter_ref_t<T_iters>...>;
	using difference_type = decltype(m_index);
	using iterator_category = std::forward_iterator_tag;

	[[nodiscard]] explicit constexpr forward_zip_iterator() noexcept
		= default; // Getting a redefinition error? T_iters mustn't be empty.

	[[nodiscard]] explicit constexpr forward_zip_iterator(T_iters&& ... args)
	: m_iters(std::forward<T_iters>(args)...)
	{ static_assert(std::forward_iterator<forward_zip_iterator<T_iters...>>); }

	constexpr forward_zip_iterator& operator++() noexcept
	{
		++m_index;
		std::apply([](auto && ... args) noexcept { ((++args),...); }, m_iters);
		return *this;
	}

	/// @note Use @c ++prefix if you don't want the temporary.
	[[nodiscard]] constexpr forward_zip_iterator operator++(int)
	{
		forward_zip_iterator tmp = *this;
		++*this;
		return tmp;
	}

	/**
	@deprecated Use a @c difference_type sentinel! This comparison is stupid
		and expensive!
	*/
	[[nodiscard]] [[deprecated("Expensive & internal. Use difference_type.")]]
	constexpr bool operator==(const forward_zip_iterator& rhs) const
	{
		const auto any_iterators_match{
			[&]<std::size_t ... i>(std::index_sequence<i...>) constexpr -> bool
			{ return ((std::get<i>(m_iters)==std::get<i>(rhs.m_iters))||...); }
		};
		return any_iterators_match(std::index_sequence_for<T_iters...>());
	}

	[[nodiscard]] constexpr
	bool operator==(const difference_type index) const noexcept
	{ return m_index == index; }

	[[nodiscard]] constexpr
	auto operator<=>(const difference_type index) const noexcept
	{ return m_index <=> index; }

	[[nodiscard]] constexpr difference_type index() const noexcept
	{ return m_index; }

	[[nodiscard]] constexpr value_type operator*() const
	noexcept( (noexcept(*T_iters{}) && ...) )
	{
		return [&]<std::size_t ... i>(std::index_sequence<i...>) -> value_type
		{
			return std::tuple(accessType(*std::get<i>(m_iters))...);
		}(std::index_sequence_for<T_iters...>());
	}
};

// sanity checks

static_assert(std::forward_iterator<forward_zip_iterator<char*>>);
static_assert(std::forward_iterator<forward_zip_iterator<const char*>>);
static_assert(
	std::sentinel_for
	<
		forward_zip_iterator<const double*>::difference_type,
		forward_zip_iterator<const double*>
	>
);

/// Utility function which returns the smallest value
[[nodiscard]] constexpr inline
zip_sentinel_t shortest(const std::integral auto& ... lengths)
{
	return std::min({static_cast<zip_sentinel_t>(lengths)...});
}

}// namespace internal

///@endcond FGL_INTERNAL_DOCS

/**
@{ @name Zip
@brief The tuple elements from @c zip are only <tt>const</tt>-qualified if the
	input iterator returns @c const .
*/

/**
@warning Undefined behavior if @p length is greater than the size of the
	smallest range.
@copydetails group-utility-zip
@param length The length of the shortest range. Must be greater than zero and
	representible by <tt>@ref fgl::zip_sentinel_t</tt>
@param ranges The ranges to zip.
@returns A @c std::ranges::subrange which represents the "zipped" ranged.
*/
template <std::ranges::forward_range ... T_ranges>
constexpr auto zip(const std::integral auto length, T_ranges&& ... ranges)
{
	FGL_DEBUG_CONSTEXPR_ASSERT(
		static_cast<zip_sentinel_t>(length)
		<= internal::shortest(std::ranges::ssize(ranges)...)
	);
	using fgl::internal::forward_zip_iterator;
	return std::ranges::subrange(
		forward_zip_iterator(std::begin(std::forward<T_ranges>(ranges))...),
		static_cast<zip_sentinel_t>(length)
	);
}

/**
@brief Determines length from the size of the smallest range.
@copydetails group-utility-zip
@param ranges The ranges to zip.
@returns A @c std::ranges::subrange which represents the "zipped" ranged.
*/
template <std::ranges::forward_range ... T_ranges>
constexpr auto zip(T_ranges&& ... ranges)
{
	return fgl::zip(
		internal::shortest(std::ranges::ssize(ranges)...),
		std::forward<T_ranges>(ranges)...
	);
}
///@} Zip

/**
@{ @name Const Zip
@brief The tuple elements from @c czip are always <tt>const</tt>-qualified.
*/

/**
@warning Undefined behavior if @p length is greater than the size of the
	smallest range.
@copydetails group-utility-zip
@param length The length of the shortest range. Must be greater than zero and
	representible by <tt>@ref fgl::zip_sentinel_t</tt>
@param ranges The ranges to zip.
@returns A @c std::ranges::subrange which represents the "zipped" ranged.
*/
template <std::ranges::forward_range ... T_ranges>
constexpr auto czip(const std::integral auto length, T_ranges&& ... args)
{
	FGL_DEBUG_CONSTEXPR_ASSERT(
		static_cast<zip_sentinel_t>(length)
		<= internal::shortest(std::ranges::ssize(args)...)
	);
	using fgl::internal::forward_zip_iterator;
	return std::ranges::subrange(
		forward_zip_iterator(std::cbegin(std::forward<T_ranges>(args))...),
		static_cast<zip_sentinel_t>(length)
	);
}

/**
@brief Determines length from the size of the smallest range.
@copydetails group-utility-zip
@param ranges The ranges to zip.
@returns A @c std::ranges::subrange which represents the "zipped" ranged.
*/
template <std::ranges::forward_range ... T_ranges>
constexpr auto czip(T_ranges&& ... args)
{
	using internal::shortest, std::ranges::ssize;
	return fgl::czip(shortest(ssize(args)...), std::forward<T_ranges>(args)...);
}
///@} Const Zip
///@} group-utility-zip

} // namespace fgl

#endif // FGL_UTILITY_ZIP_HPP_INCLUDED

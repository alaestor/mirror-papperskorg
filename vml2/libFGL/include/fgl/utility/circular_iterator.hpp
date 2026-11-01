#pragma once
#ifndef FGL_UTILITY_CIRCULAR_ITERATOR_HPP_INCLUDED
#define FGL_UTILITY_CIRCULAR_ITERATOR_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cstddef> // size_t, ptrdiff_t
#include <iterator>
#include <ranges>
#include <type_traits>

#include "../debug/constexpr_assert.hpp"

namespace fgl {

/**
@file

@example example/fgl/utility/circular_iterator.cpp
	An example for @ref group-utility-circular_iterator

@defgroup group-utility-circular_iterator Circular Iterator

@brief A "wrap-around" iterator for creating circular random-access ranges

@details @parblock
	Circular iterator is a random access iterator which models a circular
	range by employing index wrap-around behavior. By definiton, this iterator
	can never go out of bounds.

	@note While the circular iterator can never go out-of-bounds, it can still
		be invalidated.

	A circular iterator can be indexed beyond the size of the
	underlying range in both positive and negative directions to traverse
	forwards or backwards around the circle.

	@attention Due to this circular nature, <tt>iter < iter + 1</tt> may
		be @c false if <tt>iter + 1</tt> would wrap-around
		(where <tt>iter + 1</tt> would be "past-the-end" of the
		underlying range). The reverse holds true for subtraction. This also
		means that a circular iterator will never equal the end of the range.
		Special care should be taken when using a circular iterator;
		especially in loop conditions.

	@remarks To understand the wrap-around behavior, consider the following
		example: A given range has three elements, and a circular iterator is
		constructed from the beginning of the range (the zeroth element).
		The iterator is then incremented, and would then alias the first
		element. The iterator is then incremented again, and would then alias
		the second element, which is the final valid element in the range. The
		iterator is then incremented again; instead of being "past-the-end",
		it wraps around to alias the zeroth element. If the iterator is
		decremented, instead of going before the beginning, it would then
		alias the last element again. This also hold true for arithmetic:
		<tt>iter + 3</tt> would alias the zeroth element.
@endparblock

@see the example program @ref example/fgl/utility/circular_iterator.cpp
@{
*/

/**
@brief Wraps an index within the bounds of <tt>[0, range_size)</tt>.
@details The wrap-around behavior is such that index of @c -1 is
	wrapped to <tt>range_size - 1</tt> and an index of @c range_size is
	wrapped to @c 0 . This is true even if the absolute value of the @p index
	is many times larger than the @p range_size ; it can be thought of as
	"wrapping around" the range many times over again.
@param index The index to be wrapped
@param range_size The size of the range to wrap within (asserted to be greater
	than zero)
@returns The wrapped index
*/
template <std::integral T>
[[nodiscard]] constexpr inline T wrap_index(T index, const T range_size)
{
	FGL_DEBUG_CONSTEXPR_ASSERT(range_size > 0);
	if constexpr (std::signed_integral<T>)
	{
		if (index < 0)
        	index += range_size * ((0 - index) / range_size + 1);
	}

    return index % range_size;
}

/**
@copydoc group-utility-circular_iterator
@tparam T The underlying iterator type
*/
template <std::random_access_iterator T_iter>
class circular_iterator
{
	public:
	using difference_type = std::iter_difference_t<T_iter>;
	using value_type = std::iter_value_t<T_iter>;
	using reference_type = std::iter_reference_t<T_iter>;
	using iterator_category = std::random_access_iterator_tag;

	///@{ @name Constructors

	[[nodiscard]] constexpr explicit circular_iterator() = default;

	/**
	@param begin The beginning of the underlying range
	@param end The end of the underlying range (assertion: must be
		<tt>> begin</tt>)
	@param cursor The initial position within the range (assertion: must be
		<tt>>= begin</tt> and <tt>< end</tt>)
	*/
	[[nodiscard]] constexpr explicit
	circular_iterator(T_iter begin, T_iter end, T_iter cursor)
	noexcept(std::is_nothrow_copy_constructible_v<T_iter>)
	: m_begin(begin), m_end(end), m_iter(cursor)
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_begin < m_end);
		FGL_DEBUG_CONSTEXPR_ASSERT(m_iter >= m_begin);
		FGL_DEBUG_CONSTEXPR_ASSERT(m_iter < m_end);
		using namespace std::ranges;
		static_assert(std::random_access_iterator<circular_iterator<T_iter>>);
		static_assert(common_range<circular_iterator<T_iter>>);
		static_assert(random_access_range<circular_iterator<T_iter>>);
	}

	/**
	@note This is equivalent to calling
		<tt>circular_iterator(begin, end, begin)</tt>
	@param begin The beginning of the underlying range
	@param end The end of the underlying range (assertion: must be
		<tt>> begin</tt>)
	*/
	[[nodiscard]] constexpr explicit
	circular_iterator(T_iter begin, T_iter end)
	noexcept(std::is_nothrow_copy_constructible_v<T_iter>)
	: circular_iterator(begin, end, begin)
	{}

	/**
	@note This is equivalent to calling
		@code
		circular_iterator(
			std::ranges::begin(range),
			std::ranges::begin(range) + std::ranges::size(range),
			cursor
		)
		@endcode
	@param range The underlying range
	@param cursor The initial position within the range (assertion: must be
		greater than or equal to the beginning of the range and less than the
		end of the range)
	*/
	[[nodiscard]] constexpr explicit
	circular_iterator(
		std::ranges::random_access_range auto& range,
		T_iter cursor)
	noexcept(
		std::is_nothrow_copy_constructible_v<T_iter>
		&& noexcept(std::ranges::begin(range))
		&& noexcept(std::ranges::begin(range) + std::ranges::size(range))
	)
	:
		circular_iterator(
			std::ranges::begin(range),
			std::ranges::begin(range) + std::ranges::size(range),
			cursor
		)
	{}

	/**
	@note This is equivalent to calling
		<tt>circular_iterator(range, std::ranges::begin(range))</tt>
	@param range The underlying range
	@param cursor The initial position within the range (assertion: must be
		greater than or equal to the beginning of the range and less than the
		end of the range)
	*/
	[[nodiscard]] constexpr explicit
	circular_iterator(std::ranges::random_access_range auto& range)
	noexcept(noexcept(circular_iterator(range, std::ranges::begin(range))))
	: circular_iterator(range, std::ranges::begin(range))
	{}

	[[nodiscard]] constexpr
	circular_iterator(const circular_iterator& other)
	noexcept(std::is_nothrow_copy_constructible_v<T_iter>)
	: m_begin(other.m_begin), m_end(other.m_end), m_iter(other.m_iter)
	{}

	///@}

	constexpr circular_iterator& operator=(
		const circular_iterator& other) noexcept = default;

	/// @returns The beginning of the underlying range
	[[nodiscard]] constexpr T_iter begin() const noexcept { return m_begin; }

	/// @returns The end of the underlying range
	[[nodiscard]] constexpr T_iter end() const noexcept { return m_end; }

	/// @returns The current underlying iterator within the underlying range
	[[nodiscard]] constexpr T_iter cursor() const noexcept { return m_iter; }

	///@{ @name Arithmetic

	[[nodiscard]] constexpr
	circular_iterator operator+(const circular_iterator& rhs) const noexcept
	{
		return circular_iterator(
			m_begin, m_end, relative_offset(m_iter - rhs.m_iter)
		);
	}

	[[nodiscard]] constexpr
	circular_iterator operator+(const difference_type n) const noexcept
	{ return circular_iterator(m_begin, m_end, relative_offset(n)); }

	constexpr circular_iterator& operator+=(const difference_type n) noexcept
	{
		m_iter = relative_offset(n);
		return *this;
	}

	constexpr circular_iterator& operator++() noexcept
	{
		if (m_iter == m_end - 1) m_iter = m_begin;
		else ++m_iter;
		return *this;
	}

	[[nodiscard]] constexpr circular_iterator operator++(int) noexcept
	{
		circular_iterator tmp = *this;
		++*this;
		return tmp;
	}

	[[nodiscard]] constexpr
	difference_type operator-(const circular_iterator& rhs) const noexcept
	{ return wrap_index(m_iter - rhs.m_iter, m_end - m_begin); }

	[[nodiscard]] constexpr
	circular_iterator operator-(const difference_type n) const noexcept
	{ return circular_iterator(m_begin, m_end, relative_offset(-n)); }

	constexpr circular_iterator& operator-=(const difference_type n)
	noexcept
	{
		m_iter = relative_offset(-n);
		return *this;
	}

	constexpr circular_iterator& operator--() noexcept
	{
		if (m_iter == m_begin) m_iter = m_end - 1;
		else --m_iter;
		return *this;
	}

	[[nodiscard]] constexpr circular_iterator operator--(int) noexcept
	{
		circular_iterator tmp = *this;
		--*this;
		return tmp;
	}

	///@}

	///@{ @name Accessors

	[[nodiscard]] constexpr reference_type operator*() noexcept
	{ return *m_iter; }

	[[nodiscard]] constexpr const reference_type operator*() const noexcept
	{ return *m_iter; }

	[[nodiscard]] constexpr
	reference_type operator[](const difference_type index) noexcept
	{ return *(m_begin + fgl::wrap_index(index, m_end - m_begin)); }

	[[nodiscard]] constexpr
	const reference_type operator[](const difference_type index) const noexcept
	{ return *(m_begin + fgl::wrap_index(index, m_end - m_begin)); }

	///@}

	///@{ @name Comparisons

	[[nodiscard]] constexpr
	bool operator==(const circular_iterator& rhs) const noexcept
	{ return m_iter == rhs.m_iter; }

	[[nodiscard]] constexpr
	bool operator<(const circular_iterator& rhs) const noexcept
	{ return m_iter < rhs.m_iter; }

	[[nodiscard]] constexpr
	auto operator<=>(const circular_iterator&) const noexcept = default;

	///@}

private:
	T_iter m_begin{ nullptr };
	T_iter m_end{ nullptr };
	T_iter m_iter{ nullptr };

	/// @returns An iterator to an offset which is relative forwards
	[[nodiscard]] constexpr
	T_iter relative_offset(const difference_type offset) const noexcept
	{
		return m_begin + fgl::wrap_index(
			offset + (m_iter - m_begin),
			m_end - m_begin
		);
	}
};

///@relates fgl::circular_iterator
template <std::random_access_iterator T_iter>
[[nodiscard]] constexpr circular_iterator<T_iter> operator+(
	const typename circular_iterator<T_iter>::difference_type n,
	const circular_iterator<T_iter>& iter)
noexcept
{ return iter + n; }

static_assert(std::random_access_iterator<circular_iterator<char*>>);
static_assert(std::ranges::common_range<circular_iterator<char*>>);
static_assert(std::ranges::random_access_range<circular_iterator<char*>>);

// deduction guide
template <std::ranges::random_access_range T_range>
circular_iterator(T_range& range)
-> circular_iterator<decltype(std::ranges::begin(range))>;

/**
@brief Constructs a @c std::ranges::subrange from a
	@c circular_iterator and an @c std::unreachable_sentinel .
@note this is equivalent to
	@code
	std::ranges::subrange(
		fgl::circular_iterator{ range },
		std::unreachable_sentinel
	)
	@endcode
@param range The range to alias
*/
[[nodiscard]] constexpr
auto make_circular_range_alias(std::ranges::random_access_range auto& range)
noexcept(noexcept(circular_iterator{ range }))
{
	return std::ranges::subrange{
		circular_iterator{ range },
		std::unreachable_sentinel
	};
}

///@}

} // namespace fgl

#endif // FGL_UTILITY_CIRCULAR_ITERATOR_HPP_INCLUDED

#pragma once
#ifndef FGL_ENUMERATE_HPP_INCLUDED
#define FGL_ENUMERATE_HPP_INCLUDED

// For most cases, std::views:iota is what you want
// Or std::iota to fill a container
// But for needs those don't fill, there's this.

#include <concepts> // integral
#include <iterator> // forward_iterator
#include <cstddef>	// uint64_t
#include <limits>	// numeric_limits

namespace fgl {
namespace enumerate {

template <std::integral T = int64_t>
class IntegralRange
{
	class Iterator
	{
		using difference_type = T;
		using value_type = T;
		using pointer = const T*;
		using reference = const T&;
		using iterator_category = std::forward_iterator_tag;

		T m_value;
		const bool m_reverse;

	public:
		constexpr Iterator(const T value, const bool reverse)
		: m_value(value), m_reverse(reverse)
		{}

		constexpr Iterator& operator++()
		{
			m_reverse ? --m_value : ++m_value;
			return *this;
		}

		constexpr Iterator operator++(int)
		{
			Iterator retval = *this;
			++(*this);
			return retval;
		}

		constexpr bool operator==(Iterator other) const
		{
			return m_value == other.m_value;
		}

		constexpr bool operator!=(Iterator other) const
		{
			return !(*this == other);
		}

		constexpr T& operator*() { return m_value; }
	};

	const T m_from;
	const T m_to;
	const bool m_reverse;

public:
	constexpr IntegralRange(T from, T to)
	: m_from(from), m_to(to), m_reverse(m_to <= m_from)
	{}

	constexpr T from() const { return m_from; }
	constexpr T to() const { return m_to; }

	constexpr Iterator begin() const
	{
		return Iterator(m_from, m_reverse);
	}

	constexpr Iterator end() const
	{
		return Iterator((m_reverse ? m_to - 1 : m_to + 1), m_reverse);
	}
};

template <std::integral T = uint64_t>
class CountTo : public IntegralRange<T>
{
public:
	constexpr CountTo(T limit)
	: IntegralRange<T>(0, limit)
	{}
};

} // namespace enumerate
} // namespace fgl

#endif // FGL_ENUMERATE_HPP_INCLUDED

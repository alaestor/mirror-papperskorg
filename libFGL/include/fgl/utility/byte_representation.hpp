#pragma once
#ifndef FGL_UTILITY_BYTE_REPRESENTATION_HPP_INCLUDED
#define FGL_UTILITY_BYTE_REPRESENTATION_HPP_INCLUDED

#include <span>
#include <algorithm>
#include <bit>
#include <type_traits>
#include <ranges>

#include "../types/traits.hpp"

/// HAS NO TESTS YET

namespace fgl {

template <typename T>
requires std::is_trivially_copyable_v<T>
inline constexpr void write_bytes(const T& from, const std::span<std::byte, sizeof(T)> to) noexcept
{
	const auto a{ std::bit_cast<std::array<std::byte, sizeof(T)>>(from) };
	std::copy(a.begin(), a.end(), to.begin());
}

template <typename T>
requires std::is_trivially_copyable_v<T>
inline constexpr T read_bytes(const std::span<const std::byte, sizeof(T)> from) noexcept
{
	std::array<std::byte, sizeof(T)> a;
	std::copy(from.begin(), from.end(), a.begin());
	return std::bit_cast<T>(a);
}

namespace unsafe {
template <typename T>
requires std::is_trivially_copyable_v<T>
inline constexpr void write_bytes(const T& from, std::byte* to) noexcept
{ fgl::write_bytes(from, std::span<std::byte, sizeof(T)>(to, to + sizeof(T))); }

template <typename T>
requires std::is_trivially_copyable_v<T>
inline constexpr T read_bytes(const std::byte* const from) noexcept
{ return fgl::read_bytes<T>(std::span<const std::byte, sizeof(T)>(from, from + sizeof(T))); }
} // namespace unsafe


template <typename T>
requires std::is_trivially_copyable_v<T>
struct byte_representation : public std::array<std::byte, sizeof(T)>
{
	using base_t = std::array<std::byte, sizeof(T)>;

	[[nodiscard]] constexpr byte_representation(T t) noexcept
	: base_t(std::bit_cast<base_t>(t))
	{}

	constexpr auto operator<=>(const byte_representation&) const noexcept
		= default;

	// T helpers

	[[nodiscard]] constexpr T value() const noexcept
	{ return std::bit_cast<T>(static_cast<base_t>(*this)); }

	[[nodiscard]] constexpr operator T() const noexcept
	{ return value(); }

	constexpr auto operator=(const T& t) noexcept(noexcept(std::copy<
		typename base_t::const_iterator, typename base_t::iterator>))
	{
		const base_t arr{ std::bit_cast<base_t>(t) };
		std::copy(arr.cbegin(), arr.cend(), this->begin());
	}

	constexpr auto operator<=>(const T& t) const noexcept
	{ return value() <=>  t; }
};

///@}
}; // namespace fgl

#endif // FGL_UTILITY_BYTE_REPRESENTATION_HPP_INCLUDED

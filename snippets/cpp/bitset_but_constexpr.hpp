#pragma once
#ifndef BITSET_BUT_CONSTEXPR_HPP_INCLUDED
#define BITSET_BUT_CONSTEXPR_HPP_INCLUDED

/// Not recommended for use beyond the limited scope of this project!
// MisterMime project's bitset
// TODO replace entirely with future constexpr std::bitset

#include <cstddef> // size_t, byte
#include <climits> // CHAR_BIT
#include <concepts> // same_as, unsigned_integral
#include <array>
#include <cassert>

/*
	fuck :)

	std::bitset isn't fully constexpr

	https://github.com/cplusplus/papers/issues/1087
	http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2417r0.pdf


	WARNING! This is no where near a full implementation of bitset!!!!
	Only to be used here: it mimicks only a small subset of the interface
	and it does things very differently from the usual implementations!
*/

template<typename  T>
concept BitType = std::same_as<std::byte, T> || std::unsigned_integral<T>;

template <std::size_t N_BITS, BitType CHUNK_T = std::byte>
class Bitset_but_constexpr
{// TODO replace with std::bitset when P2417 is accepted (C++24?)
	static constexpr std::size_t bits_per_chunk{ sizeof(CHUNK_T) * CHAR_BIT };
	std::array<CHUNK_T, N_BITS> m_bFlags{ static_cast<CHUNK_T>(0) };

	[[nodiscard]]
	inline constexpr std::size_t index(const std::size_t bit_number) const
	{
		if (bit_number == 0)
			return 0;
		else return bit_number == 0 ? 0 : bit_number / bits_per_chunk;
	}

	[[nodiscard]]
	inline constexpr CHUNK_T bit(const std::size_t bit_number) const
	{
		return static_cast<CHUNK_T>(
			1u << (bit_number == 0 ? 0 : bit_number % bits_per_chunk)
		);
	}

	public:
	constexpr Bitset_but_constexpr() = default;

	constexpr void set(const std::size_t bit_number)
	{ // sets a bit to true
		assert(bit_number < N_BITS);
		m_bFlags[index(bit_number)] |= bit(bit_number);
	}

	constexpr void reset(const std::size_t bit_number)
	{ // sets a bit to false / 0
		assert(bit_number < N_BITS);
		m_bFlags[index(bit_number)] &= ~bit(bit_number);
	}

	constexpr void flip(const std::size_t bit_number)
	{ // toggles a bit
		assert(bit_number < N_BITS);
		m_bFlags[index(bit_number)] ^= bit(bit_number);
	}

	[[nodiscard]]
	constexpr bool test(const std::size_t bit_number) const
	{ // returns boolean state of a bit
		assert(bit_number < N_BITS);
		return static_cast<bool>(m_bFlags[index(bit_number)]&bit(bit_number));
	}

	[[nodiscard]]
	constexpr std::size_t size() const
	{ return N_BITS; }
};

#endif // BITSET_BUT_CONSTEXPR_HPP_INCLUDED

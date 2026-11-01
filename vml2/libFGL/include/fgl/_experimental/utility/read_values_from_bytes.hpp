#pragma once
#ifndef FGL_UTILITY_READ_VALUES_FROM_BYTES_HPP_INCLUDED
#define FGL_UTILITY_READ_VALUES_FROM_BYTES_HPP_INCLUDED
#include "../../environment/libfgl_compatibility_check.hpp"

#include <cstddef>
#include <cstring>
#include <cassert>
#include <concepts>

#include "../../types/range_constraints.hpp"
#include "../../types/traits.hpp"

namespace fgl {

/**
@brief Reads values in a sequential order from a byte range
@tparam T_range The type of byte range to be read from.
@tparam Ts... A sequence of integer types and/or pointers to
	trivially copyable types.
@param range The byte range to read from
@param[out] positionals If the 'positional' is an integer type, it represents
	a number of bytes to advance the read cursor before processing the next
	positional; in which case it's asserted to be greater than zero. If the
	positional is a pointer to a trivially copyable type, the size of the
	underlying type is used to copy memory from the read cursor to the address
	held by the pointer, and then advance the cursor; in which case it's
	asserted to be not @c nullptr .
*/
template <fgl::contiguous_range_byte_type T_range, typename ... Ts>
requires
	((std::integral<Ts> || fgl::traits::pointer_to_trivially_copyable<Ts>)
	&& ...)
void read_values_from_bytes(const T_range& byte_range, Ts ... positionals)
{
	auto cursor{ std::ranges::cbegin(byte_range) };

	auto process{
		[&]<typename T>(T arg) -> void
		{
			using difference_t = std::ranges::range_difference_t<T_range>;
			if constexpr (std::integral<T>)
			{
				assert(arg > 0);
				cursor += static_cast<difference_t>(arg);
			}
			else
			{
				assert(arg != nullptr);
				constexpr std::size_t size{ sizeof(std::remove_pointer_t<T>) };
				std::memcpy(arg, &*cursor, size);
				cursor += static_cast<difference_t>(size);
			}
		}
	};

	(process(positionals), ...);
}

///@}
} // namespace fgl
#endif // FGL_UTILITY_READ_VALUES_FROM_BYTES_HPP_INCLUDED

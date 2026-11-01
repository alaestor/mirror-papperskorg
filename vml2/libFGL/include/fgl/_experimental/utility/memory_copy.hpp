#pragma once
#ifndef FGL_UTILITY_MEMORY_COPY_HPP_INCLUDED
#define FGL_UTILITY_MEMORY_COPY_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cstring>
#include <type_traits>
#include <stdexcept>
#include <bit>

#include "../types/range_constraints.hpp"
#include "../types/traits.hpp"

namespace fgl {

/**
@file

@defgroup group-utility-memory_copy Memory Copy

@brief A slightly safer way to @c std::memcpy

@{
*/

namespace unsafe {

/**
@ingroup group-utility-memory_copy
@brief An unsafe wrapper for @c std::memcpy which determines the size to copy
	from the @c sizeof(T) .
@details This is equivalent to calling
	@code
	std::memcpy(destination, source, sizeof(T));
	@endcode
@warning @parblock
	As this function uses a raw byte pointer for the source, it isn't
	bounds-checked and could lead to undefined behavior if used improperly.
	Ensure that the source memory location is valid for the length
	@c sizeof(T) before using this function.

	Also be aware that this could lead to undefined behavior if the source
	was reinterpret-cast to a byte pointer form a type which is not trivially
	copyable to the @p T destination type.
	@endparblock
@param[out] destination The destination object to copy to.
@param source The source memory location to copy from.
*/
template <typename T>
requires std::is_trivially_copyable_v<T>
inline void memory_copy(T* destination, const void* source) noexcept
{
	std::memcpy(destination, source, sizeof(T));
}

} // namespace unsafe

/**
@brief A bounds-checking wrapper for @c std::memcpy .
@details Performs a bounds-check to ensure that the size of the source range is
	greater than or equal to the @c sizeof(T) . If the bounds are valid, it
	then calls @c std::memcpy . Otherwise, it throws.
@param[out] destination A pointer to the destination object to copy to.
@param source The byte range to copy from, whose size must be greater than or
	equal to the @c sizeof(T) and have a well-formed overload of
	<tt>std::ranges::data(source)</tt>
@throws std::out_of_range If the size of @p T is greater than the size of the
	source range.
*/
template <typename T, fgl::contiguous_range_byte_type T_source_range>
requires (std::is_trivially_copyable_v<T>)
	&& requires (T_source_range r){ std::ranges::data(r); }
inline void memory_copy(
	T* destination,
	const T_source_range& source_range)
{
	if (std::ranges::size(source_range) < sizeof(T)) throw std::out_of_range(
		"source range is smaller than the size of the destination type"
	);

	std::memcpy(destination, std::ranges::data(source_range), sizeof(T));
}

/**
@brief An overload for when the @p source and @p destination types are the
	same.
@details This is equivalent to calling
	@code
	std::memcpy(destination, source, sizeof(T));
	@endcode
@param[out] destination A pointer to the destination object to copy to.
@param source The source memory location to copy from.
*/
template <typename T>
requires std::is_trivially_copyable_v<T>
inline void memory_copy(T* destination, const T* source) noexcept
{ std::memcpy(destination, source, sizeof(T)); }

/**
@brief An overload for when a @c std::bit_cast assignment would be more
	appropriate than a raw @c std::memcpy .
@details This is equivalent to
	@code
	*destination = bit_cast<T_destination>(*source);
	@endcode
@param[out] destination A pointer to the destination object to copy to.
@param source The pointer to the source object to copy from.
*/
template <typename T_destination, typename T_source>
requires
	std::is_trivially_copyable_v<T_destination>
	&& std::is_trivially_copyable_v<T_source>
	&& (sizeof(T_destination) == sizeof(T_source))
	&& requires (T_destination destination, T_source source)
	{
		destination = destination;
		std::bit_cast<T_destination>(source);
	}
constexpr inline
void memory_copy(T_destination* destination, const T_source* source) noexcept
{ *destination = std::bit_cast<T_destination>(*source); }

///@}
} // namespace fgl
#endif // FGL_UTILITY_MEMORY_COPY_HPP_INCLUDED

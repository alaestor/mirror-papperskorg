#pragma once
#ifndef FGL_TYPES_CBYTE_PTR_WRAPPER_HPP_INCLUDED
#define FGL_TYPES_CBYTE_PTR_WRAPPER_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <iterator>
#include <concepts>

#include "./traits.hpp"

namespace fgl {

/**
@file

@defgroup group-types-cbyte_ptr_wrapper C Byte Pointer Conversion Wrapper

@brief A wrapper for implicit conversion between various byte pointer types.

@details @parblock
	Ideally, <tt>std::byte</tt> would be used whenever representing raw
	bytes. However, often for legacy and ABI reasons, most interfaces use
	other C byte types such as <tt>char*</tt>, <tt>unsigned char*</tt>, and
	<tt>void*</tt>. This wrapper provides implicit conversion to and from
	these byte pointer types. When an implicit conversion is required, if the
	underlying pointer is statically convertible to the new type then a
	<tt>constexpr</tt>-specified <tt>static_cast</tt> implicit conversion
	operator will be used. Otherwise, the non-<tt>constexpr</tt>-specified
	<tt>reinterpret_cast</tt> implicit conversion operator will be used.

	If the underlying pointer type is not <tt>void*</tt>, the wrapper
	satisfies the <tt>contiguous_iterator</tt> concept.

	@endparblock

@{
*/

/**
@copydoc group-types-cbyte_ptr_wrapper
@note This isn't a smart-pointer. The pointer should be non-owning!
@tparam T The underlying pointer type which must satisfy
	<tt>@ref fgl::traits::pointer_to_byte</tt>.
*/
template <fgl::traits::pointer_to_byte T>
struct cbyte_ptr_wrapper
{
	using difference_type = std::ptrdiff_t;
	using value_type = std::remove_pointer_t<T>;
	using iterator_category = std::contiguous_iterator_tag;

	T underlying_pointer{ nullptr };

	///@{ @name Constructors
	[[nodiscard]] explicit constexpr cbyte_ptr_wrapper() noexcept = default;

	/**
	@brief Constructs a byte pointer conversion wrapper
	@param non_owning_pointer The non-owning pointer which will be wrapped
	*/
	[[nodiscard]] constexpr
	cbyte_ptr_wrapper(T non_owning_pointer) noexcept
	: underlying_pointer(non_owning_pointer)
	{}

	/**
	@{ @name Implicit conversion operators
	@brief Converts the underlying pointer to @p T_cast
	@tparam T_cast The type to convert to
	@returns The converted pointer
	*/

	/// an implicit <tt>reinterpret_cast</tt> conversion
	template <fgl::traits::pointer_to_byte T_cast>
	[[nodiscard]] operator T_cast() const noexcept
	{ return reinterpret_cast<T_cast>(underlying_pointer); }

	/// an implicit <tt>constexpr</tt> <tt>static_cast</tt> conversion
	template <fgl::traits::pointer_to_byte T_cast>
	requires std::convertible_to<T, T_cast>
	[[nodiscard]] constexpr operator T_cast() const noexcept
	{ return static_cast<T_cast>(underlying_pointer); }
	///@}

	///@{ @name Dereference Operators
	[[nodiscard]] constexpr auto& operator*() const noexcept
	{ return *underlying_pointer; }

	[[nodiscard]] constexpr auto& operator[](
		const difference_type offset) const noexcept
	{ return underlying_pointer[offset]; }

	[[nodiscard]] constexpr auto& operator->() const noexcept
	{ return underlying_pointer; }
	///@}

	/**
	@{ @name Assignment Operators

	@brief Implicitly converts @p new_pointer to the underlying pointer type
		and stores it in the wrapper.
	@param new_pointer The non-owning pointer which will be wrapped
	*/

	/// Converts and assigns a byte pointer, using <tt>reinterpret_cast</tt>
	template <fgl::traits::pointer_to_byte T_other>
	cbyte_ptr_wrapper& operator=(T_other new_pointer) noexcept
	{
		underlying_pointer = reinterpret_cast<T>(new_pointer);
		return *this;
	}

	/// Converts and assigns a byte pointer, using <tt>static_cast</tt>
	template <fgl::traits::pointer_to_byte T_other>
	requires std::convertible_to<T_other, T>
	constexpr cbyte_ptr_wrapper& operator=(T_other new_pointer) noexcept
	{
		underlying_pointer = static_cast<T>(new_pointer);
		return *this;
	}
	///@}

	[[nodiscard]] constexpr cbyte_ptr_wrapper operator+(
		const difference_type offset) const noexcept
	{ return { underlying_pointer + offset }; }

	[[nodiscard]] constexpr cbyte_ptr_wrapper operator-(
		const difference_type offset) const noexcept
	{ return { underlying_pointer - offset }; }

	constexpr cbyte_ptr_wrapper& operator+=(const difference_type offset)
	noexcept
	{
		underlying_pointer += offset;
		return *this;
	}

	constexpr cbyte_ptr_wrapper& operator-=(const difference_type offset)
	noexcept
	{
		underlying_pointer -= offset;
		return *this;
	}

	/**
	@brief Subtracts the <tt>reinterpret_cast</tt>-converted @p other pointer
		from the underlying pointer.
	*/
	template <fgl::traits::pointer_to_byte T_other>
	[[nodiscard]] difference_type operator-(
		const cbyte_ptr_wrapper<T_other> other) const noexcept
	{ return underlying_pointer - reinterpret_cast<T>(other); }

	/**
	@brief Subtracts the <tt>static_cast</tt>-converted @p other pointer from
		the underlying pointer.
	*/
	template <fgl::traits::pointer_to_byte T_other>
	requires std::convertible_to<T_other, T>
	[[nodiscard]] constexpr difference_type operator-(
		const cbyte_ptr_wrapper<T_other> other) const noexcept
	{ return underlying_pointer - static_cast<T>(other); }

	/**
	@brief Subtracts the <tt>reinterpret_cast</tt>-converted @p other pointer
		from the underlying pointer.
	*/
	template <fgl::traits::pointer_to_byte T_other>
	[[nodiscard]] difference_type operator-(const T_other other)
	const noexcept
	{ return underlying_pointer - reinterpret_cast<T>(other); }

	/**
	@brief Subtracts the <tt>static_cast</tt>-converted @p other pointer from
		the underlying pointer.
	*/
	template <fgl::traits::pointer_to_byte T_other>
	requires std::convertible_to<T_other, T>
	[[nodiscard]] constexpr difference_type operator-(const T_other other)
	const noexcept
	{ return underlying_pointer - static_cast<T>(other); }

	constexpr cbyte_ptr_wrapper& operator++() noexcept
	{
		++underlying_pointer;
		return *this;
	}

	/// @note Use <tt>++prefix</tt> if you don't want the copy
	[[nodiscard]] constexpr cbyte_ptr_wrapper operator++(int) noexcept
	{
		auto copy{ *this };
		++underlying_pointer;
		return copy;
	}

	constexpr cbyte_ptr_wrapper& operator--() noexcept
	{
		--underlying_pointer;
		return *this;
	}

	/// @note Use <tt>--prefix</tt> if you don't want the copy
	[[nodiscard]] constexpr cbyte_ptr_wrapper operator--(int) noexcept
	{
		auto copy{ *this };
		--underlying_pointer;
		return copy;
	}
	///@}
};

///@} group-types-cbyte_ptr_wrapper

/**
@relates cbyte_ptr_wrapper
@{ @name Non-member Operators
*/

/**
@relates cbyte_ptr_wrapper
@brief converts both underlying pointers to <tt>const void*</tt> and
	then performs the comparison.
*/
template
<
	fgl::traits::pointer_to_byte T_lhs,
	fgl::traits::pointer_to_byte T_rhs
>
[[nodiscard]] inline constexpr bool operator==(
	const cbyte_ptr_wrapper<T_lhs> lhs,
	const cbyte_ptr_wrapper<T_rhs> rhs) noexcept
{
	return static_cast<const void*>(lhs.underlying_pointer)
		== static_cast<const void*>(rhs.underlying_pointer);
}

///@copybrief operator==() @relates cbyte_ptr_wrapper
template
<
	fgl::traits::pointer_to_byte T_lhs,
	fgl::traits::pointer_to_byte T_rhs
>
[[nodiscard]] inline constexpr auto operator<=>(
	const cbyte_ptr_wrapper<T_lhs> lhs,
	const cbyte_ptr_wrapper<T_rhs> rhs) noexcept
{
	return static_cast<const void*>(lhs.underlying_pointer)
		<=> static_cast<const void*>(rhs.underlying_pointer);
}

///@copybrief operator==() @relates cbyte_ptr_wrapper
template
<
	fgl::traits::pointer_to_byte T_wrapper_ptr,
	fgl::traits::pointer_to_byte T_ptr
>
[[nodiscard]] inline constexpr bool operator==(
	const cbyte_ptr_wrapper<T_wrapper_ptr> wrapper,
	const T_ptr pointer) noexcept
{
	return static_cast<const void*>(wrapper.underlying_pointer)
		== static_cast<const void*>(pointer);
}

///@copybrief operator==() @relates cbyte_ptr_wrapper
template
<
	fgl::traits::pointer_to_byte T_wrapper_ptr,
	fgl::traits::pointer_to_byte T_ptr
>
[[nodiscard]] inline constexpr auto operator<=>(
	const cbyte_ptr_wrapper<T_wrapper_ptr> wrapper,
	const T_ptr pointer) noexcept
{
	return static_cast<const void*>(wrapper.underlying_pointer)
		<=> static_cast<const void*>(pointer);
}

/// Compares the underlying pointer to @c nullptr  @relates cbyte_ptr_wrapper
template <fgl::traits::pointer_to_byte T_lhs>
[[nodiscard]] inline constexpr bool operator==(
	const cbyte_ptr_wrapper<T_lhs> wrapper,
	const std::nullptr_t nptr) noexcept
{
	return wrapper.underlying_pointer == nptr;
}

/// @relates cbyte_ptr_wrapper
template <fgl::traits::pointer_to_byte T>
[[nodiscard]] inline constexpr cbyte_ptr_wrapper<T> operator+(
	const std::iter_difference_t<cbyte_ptr_wrapper<T>> lhs,
	const cbyte_ptr_wrapper<T> rhs) noexcept
{
	return { lhs + rhs.underlying_pointer };
}
///@}

static_assert(std::contiguous_iterator<cbyte_ptr_wrapper<unsigned char*>>);

} // namespace fgl

#endif // FGL_TYPES_CBYTE_PTR_WRAPPER_HPP_INCLUDED




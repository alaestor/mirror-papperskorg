#pragma once
#ifndef FGL_TYPE_TRAITS_HPP_INCLUDED
#define FGL_TYPE_TRAITS_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cstddef> // byte
#include <type_traits>
#include <ranges>
#include <ostream>
#include <chrono>
#include <concepts>

namespace fgl::traits {

/**
@file

@defgroup group-types-traits Type Traits

@brief Types, traits, and concepts

@{
*/

/// Satisfied if @p T is a @c std::chrono::is_clock_v .
template <class T>
concept clock = std::chrono::is_clock_v<T>;

/**
@brief Satisfied if @p T is a <tt>@ref clock</tt> and @c T::is_steady .
*/
template <class T>
concept steady_clock = clock<T> && T::is_steady;

/**
@brief Satisfied if @p T is either a <tt>std::byte</tt>, <tt>char</tt>,
	or <tt>unsigned char</tt>.
*/
template <typename T>
concept byte_type =
	std::same_as<std::remove_cv_t<T>, std::byte>
	|| std::same_as<std::remove_cv_t<T>, char>
	|| std::same_as<std::remove_cv_t<T>, unsigned char>
; // no one should use signed char for raw bytes...

/// Satisfied if @p T is an integer or floating point type.
template <typename T>
concept numeric_type = std::integral<T> || std::floating_point<T>;

/// Satisfied if @p T is a pointer.
template <typename T>
concept pointer_type = std::is_pointer_v<T>;

/// Satisfied if @p T can be compared to @c nullptr with @c == and @c != .
template <typename T>
concept null_comparable = requires (T t) {
	//std::equality_comparable_with<T, std::nullptr_t>; silly C++  :(
	//https://stackoverflow.com/questions/66937947/why-is-unique-ptr-not-equality-comparable-with-nullptr-t-in-c20
	{ t == nullptr } -> std::same_as<bool>;
	{ t != nullptr } -> std::same_as<bool>;
};


/// Satisfied if @p T1 and @p T2 are not the same.
template <typename T1, typename T2>
concept not_same_as = (std::same_as<T1, T2> == false);

/**
@brief Satisfied if @p T_from is convertible to the
	<tt>std::common_type_t<T_from, T_commons...></tt>
@tparam T_from The type to convert from
@tparam T_commons... Types which, alogn with <tt>T_from</tt>, are convertible
	to the common type.
*/
template <typename T_from, typename ... T_commons>
concept convertible_to_common =
	std::convertible_to<T_from, std::common_type_t<T_from, T_commons...>>;

/// Satisfied if <tt>std::ostream& operator<<(std::ostream&, T)</tt> is defined
template <typename T>
concept printable =	requires (std::ostream& os, T const& t)
{ { os << t } -> std::same_as<std::ostream&>; };

///@cond FGL_INTERNAL_DOCS
namespace internal {
/**
@internal @brief Implementation for <tt>remove_all_pointers_t</tt>
@details credit to [klaus triendl](https://stackoverflow.com/a/39492671)
*/
template <typename T>
struct remove_all_pointers : std::conditional_t
<
	std::is_pointer_v<T>,
	remove_all_pointers<std::remove_pointer_t<T>>,
	std::type_identity<T>
>
{};
} // namespace internal
///@endcond

/// Removes all pointers from @p T such that @c T**** becomes @c T
template <typename T>
using remove_all_pointers_t = typename internal::remove_all_pointers<T>::type;

/**
@brief Removes the pointer type, and then removes any <tt>const</tt> or
	<tt>volatile</tt> qualifiers from @p T such that
	<tt>const volatile * const volatile <i>T</i></tt> becomes
	<tt><i>T</i></tt>
*/
template <typename T>
using remove_cvptr_t = typename std::remove_cv_t<std::remove_pointer_t<T>>;

/**
@brief Satisfied if @p T is a pointer (cv-removed) to <tt>void</tt> or a
	<tt>@ref fgl::traits::byte_type</tt>
@note it's common for C interfaces to use <tt>void*</tt> to represent raw
	bytes. This must be checked separately. For this usecase, use
	<tt>@ref fgl::traits::pointer_to_byte_or_void</tt>
*/
template <typename T>
concept pointer_to_byte =
	std::is_pointer_v<T>
	&& (
		byte_type<std::remove_pointer_t<T>>
		|| std::is_void_v<remove_cvptr_t<T>>
	)
;

/// Satisfied if @p T is a pointer to anything but <tt>void</tt>
template <typename T>
concept pointer_to_non_void =
	std::is_pointer_v<T> && (!std::is_void_v<remove_cvptr_t<T>>)
;

/// Satisfied if @p T is <tt>std::is_trivially_copyable</tt>
template <typename T>
concept trivially_copyable = std::is_trivially_copyable_v<T>;

/**
@brief Satisfied if @p T is a pointer to a
	<tt>@ref fgl::traits::trivially_copyable</tt> type.
*/
template <typename T>
concept pointer_to_trivially_copyable =
	std::is_pointer_v<T> && trivially_copyable<std::remove_pointer_t<T>>
;

/// <tt>true</tt> is @p T is a <tt>const</tt>-qualified reference
template <typename T>
inline constexpr bool is_const_ref{
	std::is_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>
};

/**
@brief <tt>true</tt> if @p T is a mutable, non-<tt>const</tt>-qualified
	reference
*/
template <typename T>
inline constexpr bool is_nonconst_ref{
	std::is_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>
};

/**
@brief Adds a <tt>const</tt>-qualified lvalue refernece such that
	@p T becomes <tt>const T&</tt>
*/
template <typename T>
using add_const_lvref_t = std::add_const_t<std::add_lvalue_reference_t<T>>;

/**
@brief  Adds a <tt>const</tt>-qualified rvalue reference such that
	@p T becomes <tt>const T&&</tt>
*/
template <typename T>
using add_const_rvref_t = std::add_const_t<std::add_rvalue_reference_t<T>>;

///@} group-types-traits
} // namespace fgl::traits

#endif // FGL_TYPE_TRAITS_HPP_INCLUDED

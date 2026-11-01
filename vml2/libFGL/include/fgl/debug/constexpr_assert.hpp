#pragma once
#ifndef FGL_DEBUG_CONSTEXPR_ASSERT_HPP_INCLUDED
#define FGL_DEBUG_CONSTEXPR_ASSERT_HPP_INCLUDED

#include <type_traits> // is_constant_evaluated
#include <cassert>

#include "../environment/libfgl_compatibility_check.hpp"

/**
@file

@example example/fgl/debug/constexpr_assert.cpp
	An example for @ref group-debug-constexpr_assert

@defgroup group-debug-constexpr_assert Constexpr Assert

@brief An <tt>assert</tt>-like macro with well-defined behavior for
	compile-time evaluation.

@remarks
	Calling non-<tt>constexpr</tt> functions at compile-time is "ill-formed,
	no diagnostic required". Nearly all implementations of <tt>assert</tt>
	will call an implementation-defined built-in function if the assertion
	fails. This means that the compiler isn't <i>required</i> to stop
	compilation and emit an error when <tt>assert(false);</tt> is evaluated at
	compile-time. Of couirse, nearly all compilers will fail compilation and
	emit an error, but they aren't <b>required to</b> by the standard; this
	exists to guarentee that behavior. Unlike <tt>assert(false)</tt>, a
	compile-time evaluated <tt>constexpr_assert(false)</tt> guarentees
	compilation failure and should cause the compiler to emit a relevant
	diagnostic message.

	@see the example program @ref example/fgl/debug/constexpr_assert.cpp

@todo future module overhaul will still need a .h for macros
@todo future C++ static reflection features may allow for a complete overhaul
@{
*/

#ifdef NDEBUG
	#ifndef FGL_DEBUG_CONSTEXPR_ASSERT
		#define FGL_DEBUG_CONSTEXPR_ASSERT(assertion)
	#else
		#error FGL_DEBUG_CONSTEXPR_ASSERT already defined
	#endif // ifndef FGL_DEBUG_CONSTEXPR_ASSERT
#else
	#ifndef FGL_DEBUG_CONSTEXPR_ASSERT
	// not indenting for error message readability
	/**
	@copybrief group-debug-constexpr_assert

	@details If <tt>constexpr_assert</tt> is evaluated at runtime, the
		expression is evaluated within a standard <tt>assert</tt>, but if
		it's evaluated at compile-time and the expression is resolves to
		<tt>false</tt>, then a compile-time error will be generated via
		an empty <tt>throw</tt> (which is explicitly illegal in a
		compile-time context).

	@see @ref page-fgl-macros

	@note Just like the standard <tt>assert</tt>, <tt>constexpr_assert</tt>
		can be disabled by defining <tt>NDEBUG</tt> before inclusion. The
		expression will not be evaluated.

	@param assertion An expression which is evaluated as boolean
		<tt>true</tt> or <tt>false</tt> (via <tt>static_cast</tt>).
	*/
	#define FGL_DEBUG_CONSTEXPR_ASSERT(assertion) \
	if (std::is_constant_evaluated())\
	{/* ignore warning that occurs in noexcept functions */\
	_Pragma("GCC diagnostic push")\
	_Pragma("GCC diagnostic ignored \"-Wterminate\"")\
	_Pragma("GCC diagnostic ignored \"-Wuseless-cast\"")\
	if (static_cast<bool>(assertion) == false)\
	{ throw; } /* COMPILE-TIME ASSERTION FAILED */\
	_Pragma("GCC diagnostic pop")\
	}\
	else assert((assertion));
	#else
		#error FGL_DEBUG_CONSTEXPR_ASSERT already defined
	#endif // ifndef FGL_DEBUG_CONSTEXPR_ASSERT
#endif // ifdef NDEBUG else

/**
@{ @name Opt-in Short Macros
@ref page-fgl-macros
*/
#ifdef FGL_SHORT_MACROS
	/// The Opt-in short macro symbol
	#define FGL_DEBUG_CONSTEXPR_ASSERT_SHORT_MACROS
#endif // ifdef FGL_SIMPLE_MACROS

#ifdef FGL_DEBUG_CONSTEXPR_ASSERT_SHORT_MACROS
	#ifndef constexpr_assert
		/// Alias for <tt>FGL_DEBUG_CONSTEXPR_ASSERT()<tt>
		#define constexpr_assert(assertion) \
			FGL_DEBUG_CONSTEXPR_ASSERT(assertion)
	#else
		#error constexpr_assert already defined
	#endif // constexpr_assertS
#endif // ifdef FGL_DEBUG_CONSTEXPR_ASSERT_SHORT_MACRO
///@} Opt-in Short Macros

#endif // ifndef FGL_DEBUG_CONSTEXPR_ASSERT_HPP_INCLUDED
///@} group-debug-constexpr-assert

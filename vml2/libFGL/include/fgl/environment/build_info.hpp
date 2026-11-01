#pragma once
#ifndef FGL_ENVIRONMENT_BUILD_INFO_HPP_INCLUDED
#define FGL_ENVIRONMENT_BUILD_INFO_HPP_INCLUDED
#include "./libfgl_compatibility_check.hpp"

namespace fgl {

/**
@file

@example example/fgl/debug/build_info.cpp
	An example for @ref group-environment-build_info

@defgroup group-environment-build_info Build Information

@brief Build environment detection

@details
	This header provides a way to detect the build environment. In most cases,
	it provides language level aliases for the build environment preprocessor
	symbols
@warning
	Include order and location matters. For example: variables which depend on
	macros such as <tt>NDEBUG</tt> are only a valid reflection of the program
	state at the point of inclusion because the symbols may be defined,
	un-defined, or re-defined at any point before or after inclusion of this
	header which may make the boolean state invalid (not an accurate reflection
	of the the macro). For this reason, usage of <tt>#ifdef</tt> and
	<tt>#ifndef</tt> is recommended in translation units which may
	<tt>#define</tt> and <tt>#undef<tt> preprocessor symbols frequently, or
	with unpredictable header ordering.
@{
*/

/**
@{ @name Build State
@brief Used to determine the state of the current build environment thru
	detection of the <tt>NDEBUG</tt> preprocessor symbol.
@note As a sanity-check, an <tt>#error</tt> will be generated if both
	<tt>NDEBUG</tt> and <tt>DEBUG</tt> are defined simultaneously.
*/

#if defined(NDEBUG) && defined(DEBUG)
	#error conflicting state: both NDEBUG and DEBUG are defined simultaneously
#endif

/// <tt>true</tt> if the <tt>NDEBUG</tt> preprocessor symbol is not defined
[[maybe_unused]] inline constexpr bool debug_build{
#ifndef NDEBUG
	true
#else
	false
#endif
};

/**
@brief <tt>true</tt> if the <tt>DEBUG</tt> preprocessor symbol is defined.
@note This state is the negation of <tt>fgl::debug_build</tt>, and reflects the
	presense of the <tt>NDEBUG</tt> symbol. It doesn't reflect the presense or
	absense of the commonly used, but non-standard, <tt>DEBUG</tt> symbol.
*/
[[maybe_unused]] inline constexpr bool release_build{ not debug_build };

///@} group-environment-build_info

} // namespace fgl

#endif // FGL_ENVIRONMENT_BUILD_INFO_HPP_INCLUDED

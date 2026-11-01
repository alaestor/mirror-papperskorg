#pragma once
#ifndef FGL_ENVIRCONMENT_COMPATIBILITY_CHECK_HPP_INCLUDED
#define FGL_ENVIRCONMENT_COMPATIBILITY_CHECK_HPP_INCLUDED

/**
@file

@defgroup group-environment-libfgl_compatibility_check libFGL Compatibility Check

@brief Environment compatibility checks

@details
	<tt>#include <fgl/environment/libfgl_compatibility_check.hpp></tt>

	This header is included by nearly everything in libFGL. It performs
	preprocessor checks to ensure an <i>officially compatible</i> environment;
	a minimum C++ language specification and compiler version.

	The official build environment compatibility requirments are as follows:

	- C++20 or higher
	- GCC 12.0 or higher

	@note The official compatibility requirements are more like a "reference
		environment" requirement. The minimum requirements for partial libFGL
		compatibility are much less stringent. Refer to the remarks.

	Users of libFGL may bypass compatibility checks at their sole discretion.

	- All checks can be bypassed by defining
		<tt>FGL_ACKNOWLEDGE_INCOMPATIBLE_ENVIRONMENT</tt>

	- Language version checks can be bypassed by defining
		<tt>FGL_ACKNOWLEDGE_INCOMPATIBLE_STANDARD</tt>

	- Compiler version checks can be bypassed by defining
		<tt>FGL_ACKNOWLEDGE_INCOMPATIBLE_COMPILER</tt>

@remarks
	@parblock
	<b>The constraints on official compatibility are extremely narrow.</b> The
	true viability of an environment will vary. At the time of writing,
	implementation support for C++20 features is spotty at best; especially
	with regards to compile-time STL evaluation support, which libFGL relies on
	as both a feature and a testing tool. Both components and their tests make
	heavy use of <tt>constexpr</tt>. Some tests may not compile or function
	properly within 'incompatible' environments despite some of the components
	themselves being perfectly viable in a runtime state.

	<b>The minimum requirements for partial functionality are more
	flexible.</b> For example, GCC is required for official compatibility only
	because that happens to be what our team currently works with, and GCC
	warnings are the only compiler warnings which libFGL will omit in cases
	where they are expected and acceptable. While virtually all libFGL
	components will likely function perfectly well with a version of MSVC
	which supports C++20; it's deemed 'incompatible' only because we haven't
	bothered to verify compatibility and implement MSVC warning bypasses.
	@endparblock
*/

#ifndef FGL_ACKNOWLEDGE_INCOMPATIBLE_ENVIRONMENT

// check standard version
#ifndef FGL_ACKNOWLEDGE_INCOMPATIBLE_STANDARD
	#if (__cplusplus < 202002L)
		#error "libFGL requires C++20 or higher. Define FGL_ACKNOWLEDGE_INCOMPATIBLE_STANDARD to bypass this check."
	#endif
#endif // FGL_ACKNOWLEDGE_INCOMPATIBLE_STANDARD

// Compiler checks
#ifndef FGL_ACKNOWLEDGE_INCOMPATIBLE_COMPILER
	#ifdef __GNUC__
		#define FGL_INTERNAL_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
		#if (FGL_INTERNAL_GCC_VERSION < 120001L)
			#error "libFGL requires GCC 12.0.1 or higher. Define FGL_ACKNOWLEDGE_INCOMPATIBLE_COMPILER to bypass this check."
		#endif
	#else
		#error "libFGL requires GCC 12.0.1 or higher. Define FGL_ACKNOWLEDGE_INCOMPATIBLE_COMPILER to bypass this check."
	#endif
#endif // FGL_ACKNOWLEDGE_INCOMPATIBLE_COMPILER

#endif // FGL_ACKNOWLEDGE_INCOMPATIBLE_ENVIRONMENT

#endif // FGL_ENVIRCONMENT_COMPATIBILITY_CHECK_HPP_INCLUDED

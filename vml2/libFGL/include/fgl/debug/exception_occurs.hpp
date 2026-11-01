#pragma once
#ifndef FGL_DEBUG_EXCEPTION_OCCURS_HPP_INCLUDED
#define FGL_DEBUG_EXCEPTION_OCCURS_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

/**
@file

@example example/fgl/debug/exception_occurs.cpp
	An example for @ref group-debug-exception_occurs

@defgroup group-debug-exception_occurs Exception Occurs

@brief Exception-to-bool conversion macros

@details
	Exception-to-bool conversion fascilitated by an immediate try-catch lambda
	wrapper. Useful for testing and "inline" exception handling.

	@remarks Error handling designs which rely on exception-to-bool conversions
	are almost always bad. Aside from making exception tests easier to read,
	this should only be employed in niche circumstances and not used as an
	error-handling design strategy. With that said; while exceptions should be
	exceptional, some libraries are stupid and this can be used to help
	encapsulate their stupidity in a relatively clean and readable way.

	@see the example program @ref example/fgl/debug/exception_occurs.cpp
@{
*/

#ifndef FGL_DEBUG_EXCEPTION_OCCURS
	/**
	@brief An immediate lambda wrapper which catches any exception
	@param expression An expression which will be evaluated within a
		<tt>try</tt> block.
	@returns <tt>true</tt> if an exception was caught, otherwise
		<tt>false</tt>.
	*/
	#define FGL_DEBUG_EXCEPTION_OCCURS(expression) \
		([&]() -> bool {\
		try{ expression; return false; }\
		catch(...){ return true; }}())
#else
	#error FGL_DEBUG_EXCEPTION_OCCURS already defined
#endif // ifndef FGL_DEBUG_EXCEPTION_OCCURS

#ifndef FGL_DEBUG_SPECIFIC_EXCEPTION_OCCURS
	/**
	@brief An immediate lambda wrapper which only catches a specific exception.
	@param exception The specific exception type to <tt>catch</tt>.
	@param expression An expression which will be evaluated within a
		<tt>try</tt> block.
	@note Caught exceptions which aren't the specified <tt>exception</tt> are
		re-thrown.
	*/
	#define FGL_DEBUG_SPECIFIC_EXCEPTION_OCCURS(exception, expression) \
		([&]() -> bool {\
		try{ expression; return false; }\
		catch(const exception &){ return true; }\
		catch(...){ throw; }}())
#else
	#error FGL_DEBUG_SPECIFIC_EXCEPTION_OCCURS already defined
#endif // ifndef FGL_DEBUG_SPECIFIC_EXCEPTION_OCCURS

/**
@{ @name Opt-in Short Macros
@ref page-fgl-macros
*/
#ifdef FGL_SHORT_MACROS
	/// The Opt-in short macro symbol
	#define FGL_DEBUG_EXCEPTION_OCCURS_SHORT_MACROS
#endif // ifdef FGL_SIMPLE_MACROS

#ifdef FGL_DEBUG_EXCEPTION_OCCURS_SHORT_MACROS
	#ifndef exception_occurs
		/// Alias for <tt>@ref FGL_DEBUG_EXCEPTION_OCCURS()</tt>
		#define exception_occurs(expression) \
			FGL_DEBUG_EXCEPTION_OCCURS(expression)
	#else
		#error exception_occurs already defined
	#endif // exception_occurs
	#ifndef specific_exception_occurs
		/// Alias for <tt>@ref FGL_DEBUG_SPECIFIC_EXCEPTION_OCCURS()</tt>
		#define specific_exception_occurs(exception, expression) \
			FGL_DEBUG_SPECIFIC_EXCEPTION_OCCURS(exception, expression)
	#else
		#error specific_exception_occurs already defined
	#endif // specific_exception_occurs
#endif // ifdef FGL_DEBUG_EXCEPTION_OCCURS_SHORT_MACROS
///@} opt-in short macros

///@} group-debug-exception_occurs

#endif // FGL_DEBUG_EXCEPTION_OCCURS_HPP_INCLUDED

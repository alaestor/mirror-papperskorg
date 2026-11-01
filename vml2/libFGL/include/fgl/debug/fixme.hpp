#pragma once
#ifndef FGL_DEBUG_FIXME_HPP_INCLUDED
#define FGL_DEBUG_FIXME_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <string_view>
#include <source_location>

#include "../debug/output.hpp"

namespace fgl::debug {

/**
@file

@example example/fgl/debug/fixme.cpp
	An example for @ref group-debug-fixme

@defgroup group-debug-fixme FixMe

@brief Sends reminders or code expressions to libFGL's @ref group-debug-output

@details
	@see the example program @ref example/fgl/debug/fixme.cpp

@todo future module overhaul will still need a .h for macros
@todo future C++ static reflection features may allow for a complete overhaul
@{
*/

#ifdef NDEBUG
	#ifndef FGL_DEBUG_FIX
		#define FGL_DEBUG_FIX(message)
	#else
		#error FGL_DEBUG_FIX already defined
	#endif
	#ifndef FGL_DEBUG_FIX_ME
		#define FGL_DEBUG_FIX_ME
	#else
		#error FGL_DEBUG_FIX_ME already defined
	#endif
	#ifndef FGL_DEBUG_FIX_THIS
		#define FGL_DEBUG_FIX_THIS(expr) expr
	#else
		#error FGL_DEBUG_FIX_THIS already defined
	#endif
#else
	#ifndef FGL_DEBUG_FIX
		/**
		@brief Send a message with source location info to libFGL's
			@ref group-debug-output on the "fixme" channel
		@param message a message to be sent (<tt>std::string_view</tt>)
		@note If <tt>NDEBUG</tt> is defined, this macro expands to nothing and
			no output will be sent.
		*/
		#define FGL_DEBUG_FIX(message) \
			fgl::debug::output(fgl::debug::fixme(message));
	#else
		#error FGL_DEBUG_FIX already defined
	#endif // ifndef FGL_DEBUG_FIX

	#ifndef FGL_DEBUG_FIX_ME
		/**
		@brief Send the source location info to libFGL's
			@ref group-debug-output on the "fixme" channel
		@note If <tt>NDEBUG</tt> is defined, this macro expands to nothing and
			no output will be sent.
		*/
		#define FGL_DEBUG_FIX_ME FGL_DEBUG_FIX("")
	#else
		#error FGL_DEBUG_FIX_ME already defined
	#endif // ifndef FGL_DEBUG_FIX_ME

	#ifndef FGL_DEBUG_FIX_THIS
		#ifndef FGL_DEBUG_FIX_THIS_IMPL
			/// @cond FGL_INTERNAL_DOCS
			/// @internal @brief implementation to stringify the expression
			#define FGL_DEBUG_FIX_THIS_IMPL(expr) FGL_DEBUG_FIX(#expr) expr;
			///@endcond
		#else
			#error FGL_DEBUG_FIX_THIS_IMPL already defined
		#endif // ifndef FGL_DEBUG_FIX_THIS_IMPL
		/**
		@brief Send an expression with source location info and it's
			evaluated result to libFGL's @ref group-debug-output on the
			"fixme" channel
		@param expression The expression which will be stringified
		@note If <tt>NDEBUG</tt> is defined, this macro expands to the in-place
			expression which will always be evaluated. No output will be sent.
		*/
		#define FGL_DEBUG_FIX_THIS(expression) \
			FGL_DEBUG_FIX_THIS_IMPL(expression)
	#else
		#error FGL_DEBUG_FIX_THIS already defined
	#endif // ifndef FGL_DEBUG_FIX_THIS
#endif // ifndef NDEBUG

/**
@{ @name Opt-in Short Macros
@ref page-fgl-macros
*/
#ifdef FGL_SHORT_MACROS
	/// The Opt-in short macro symbol
	#define FGL_DEBUG_FIXME_SHORT_MACROS
#endif // FGL_SHORT_MACROS

#ifdef FGL_DEBUG_FIXME_SHORT_MACROS
	#ifndef FIX
		/// Alias for <tt>@ref FGL_DEBUG_FIX()</tt>
		#define FIX(message) FGL_DEBUG_FIX(message)
	#else
		#error FIX already defined (FGL_DEBUG_FIXME_SHORT_MACROS)
	#endif // ifndef FIX

	#ifndef FIX_ME
		/// Alias for <tt>@ref FGL_DEBUG_FIX_ME()</tt>
		#define FIX_ME FGL_DEBUG_FIX_ME
	#else
		#error FIX_ME already defined (FGL_DEBUG_FIXME_SHORT_MACROS)
	#endif // ifndef FIX_ME

	#ifndef FIX_THIS
		/// Alias for <tt>@ref FGL_DEBUG_FIX_THIS()</tt>
		#define FIX_THIS(expr) FGL_DEBUG_FIX_THIS_IMPL(expr)
	#else
		#error FIX_THIS already defined (FGL_DEBUG_FIXME_SHORT_MACROS)
	#endif // ifndef FIX_THIS
#endif // ifdef FGL_DEBUG_FIXME_SHORT_MACROS
///@} Opt-in Short Macros

/**
@brief Used to specialize <tt>fgl::debug::output_config<fixme></tt> is sent to
	the <tt>@ref fgl::debug::output()</tt> when a fix-me message is emitted.
@remarks In the vast majority of cases, this object type will only be used in
	the implementations of the <tt>@ref FIX()<tt>, <tt>@ref FIX_ME()<tt>, and
	<tt>@ref FIX_THIS</tt> macros. You probably don't want to use this
	directly, but could if so desired.
@internal @note (internal) code duplication with echo.hpp
*/
struct fixme final
{
	const std::string_view string;
	const std::source_location location;

	[[nodiscard]] constexpr explicit fixme(
		const std::string_view in_string,
		const std::source_location& in_location =
			std::source_location::current())
	: string{ in_string }, location{ in_location }
	{}
};

///@cond FGL_INTERNAL_DOCS
namespace internal {
static inline constexpr string_literal fixme_cn{ "FIXME" };
} // namespace internal
///@endcond FGL_INTERNAL_DOCS

/**
@brief An <tt>fgl::debug::output_handler</tt> specialization for using
	<tt>@ref fixme</tt> with libFGL's @ref group-debug-output.

@see @ref group-debug-output and <tt>@ref fgl::debug::output::operator()()</tt>
*/
template <>
class output_config<fixme> final
:public simple_output_channel<true, priority::debug, internal::fixme_cn, fixme>
{
	output_config(auto&&...) = delete; ///< should never be instantiated
	public:
	using channel_t = simple_output_channel<
		true, priority::debug, internal::fixme_cn, fixme
	>;

	///@{ @name Configurable formatters
	/// Formatter for fix-me strings (both expressions and messages)
	static inline output::format_msg_src_t formatter{
		output::default_fmt_msg_src
	};
	///@} Configurable formatters

	/**
	@brief To satisfy <tt>fgl::debug::output_formatter</tt>
	@returns the result of <tt>formatter(fixme.string, fixme.location)</tt>
	*/
	[[nodiscard]] static std::string format(const fixme& fixme)
	{
		return formatter(fixme.string, fixme.location);
	}
};

static_assert(output_handler<output_config<fixme>, fixme>);

///@} // group-debug-fixme
} // namespace fgl::debug

#endif // ifndef FGL_DEBUG_FIXME_HPP_INCLUDED

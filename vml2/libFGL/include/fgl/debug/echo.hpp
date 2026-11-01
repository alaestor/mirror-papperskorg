#pragma once
#ifndef FGL_DEBUG_ECHO_HPP_INCLUDED
#define FGL_DEBUG_ECHO_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <string_view>
#include <sstream>
#include <source_location>
#include <utility>

#include "../debug/output.hpp"

namespace fgl::debug {

/**
@file

@example example/fgl/debug/echo.cpp
	An example for @ref group-debug-echo

@defgroup group-debug-echo Echo

@brief Send messages or code expressions with their evaluated result to
	libFGL's @ref group-debug-output

@details
	A simple way to output a message, or an expression with its result, to the
	@ref group-debug-output. Echo can also be used as a nice replacement for
	quick and dirty "print debugging".

	@see the example program @ref example/fgl/debug/echo.cpp

@todo future module overhaul will still need a .h for macros
@todo future C++ static reflection features may allow for a complete overhaul
@{
*/

#ifdef NDEBUG
	#ifndef FGL_DEBUG_ECHO
		#define FGL_DEBUG_ECHO(message)
	#else
		#error FGL_DEBUG_ECHO already defined
	#endif // ifndef FGL_DEBUG_ECHO
	#ifndef FGL_DEBUG_ECHOV
		#define FGL_DEBUG_ECHOV(expr) expr
	#else
		#error FGL_DEBUG_ECHOV already defined
	#endif // ifndef FGL_DEBUG_ECHOV
#else
	#ifndef FGL_DEBUG_ECHO
		/**
		@brief Send a message with source location info to libFGL's
			@ref group-debug-output on the "echo" channel
		@param message a message to be echoed (<tt>std::string_view</tt>)
		@note If <tt>NDEBUG</tt> is defined, this macro expands to nothing and
			no output will be sent.
		*/
		#define FGL_DEBUG_ECHO(message) \
			fgl::debug::internal::emit_echo(message);
	#else
		#error FGL_DEBUG_ECHO already defined
	#endif // ifndef FGL_DEBUG_ECHO

	#ifndef FGL_DEBUG_ECHOV
		#ifndef FGL_DEBUG_ECHOV_IMPL
			/// @cond FGL_INTERNAL_DOCS
			/// @internal @brief implementation to stringify the expression
			#define FGL_DEBUG_ECHOV_IMPL(expr) \
				fgl::debug::internal::emit_echov(expr, #expr);
			/// @endcond
		#else
			#error FGL_DEBUG_ECHOV_IMPL already defined
		#endif // ifndef FGL_DEBUG_ECHOV_IMPL
		/**
		@brief Send an expression with source location info and it's
			evaluated result to libFGL's @ref group-debug-output on the
			"echo" channel
		@note the expression is evaluated as an argument in a function call
		@param expression The expression which will be stringified and
		 	evaluated. The result of the expression will be echoed.
		@note If <tt>NDEBUG</tt> is defined, this macro expands to the in-place
			expression which will always be evaluated. No output will be sent.
		*/
		#define FGL_DEBUG_ECHOV(expression) FGL_DEBUG_ECHOV_IMPL(expression)
	#else
		#error FGL_DEBUG_ECHOV already defined
	#endif // ifndef FGL_DEBUG_ECHOV
#endif // ifndef NDEBUG

/**
@{ @name Opt-in Short Macros
@ref page-fgl-macros
*/
#ifdef FGL_SHORT_MACROS
	/// The Opt-in short macro symbol
	#define FGL_DEBUG_ECHO_SHORT_MACROS
#endif // FGL_SHORT_MACROS

#ifdef FGL_DEBUG_ECHO_SHORT_MACROS
	#ifndef ECHO
		/// Alias for <tt>FGL_DEBUG_ECHO()</tt>
		#define ECHO(message) FGL_DEBUG_ECHO(message)
	#else
		#error ECHO already defined (FGL_DEBUG_ECHO_SHORT_MACROS)
	#endif // ifndef ECHO

	#ifndef ECHOV
		/// Alias for <tt>FGL_DEBUG_ECHOV()</tt>
		#define ECHOV(expr) FGL_DEBUG_ECHOV(expr)
	#else
		#error ECHOV already defined (FGL_DEBUG_ECHO_SHORT_MACROS)
	#endif // ifndef ECHOV
#endif // ifdef FGL_DEBUG_ECHO_SHORT_MACROS
///@} opt-in short macros

/**
@brief Used to specialize <tt>fgl::debug::output_config<echo></tt> and send to
	<tt>fgl::debug::output()</tt> when an echo is emitted.
@remarks In the vast majority of cases, this object type will only be used in
	the implementations of the <tt>@ref ECHO()</tt> and <tt>@ref ECHOV()</tt>
	macros. You probably don't want to use this directly, but could if so
	desired.
@internal @note (internal) code duplication with @ref fixme.hpp
*/
struct echo
{
	const std::string string;
	const std::source_location location;

	[[nodiscard]] explicit echo(
		std::string_view in_string,
		const std::source_location in_location =
			std::source_location::current())
	: string(in_string), location(in_location)
	{}
};

///@cond FGL_INTERNAL_DOCS
namespace internal {
static inline constexpr string_literal echo_cn{ "ECHO" };
} // namespace internal
///@endcond FGL_INTERNAL_DOCS

/**
@brief An <tt>@ref fgl::debug::output_handler</tt> specialization for using
	<tt>@ref echo</tt> with libFGL's @ref group-debug-output.

@see @ref group-debug-output and @ref fgl::debug::output::operator()()
*/
template <>
class output_config<echo> final
:public simple_output_channel<true, priority::debug, internal::echo_cn, echo>
{
	output_config(auto&&...) = delete; ///< should never be instantiated
	public:
	using channel_t = simple_output_channel<
		true, priority::debug, internal::echo_cn, echo
	>;

	///@{ @name Default Formatters

	/// Default formatter for the result of <tt>@ref ECHOV()</tt>'s expression
	template <typename T>
	[[nodiscard]] static std::string default_fmt_value(const T& value)
	{
		std::stringstream ss;
		ss << " == " << value;
		return ss.str();
	}

	/**
	@brief Default formatter for <tt>@ref ECHO()</tt>'s messages and
		<tt>@ref ECHOV()</tt>'s expressions
	*/
	[[nodiscard]] static std::string default_echo_fmt(
		const std::string_view expression)
	{ return std::string(expression); }
	///@} Default Formatters

	///@{ @name Configurable formatters

	/// Formatter for <tt>@ref ECHO()</tt>'s message string @showinitializer
	static inline
	std::function<std::string(std::string_view)> message_formatter{
		default_echo_fmt
	};

	/**
	@brief Formatter for <tt>@ref ECHOV()</tt>'s expression string
	@showinitializer
	*/
	static inline
	std::function<std::string(std::string_view)> expression_formatter{
		default_echo_fmt
	};

	/**
	@brief Formatter for the result of <tt>@ref ECHOV()</tt>'s expression
	@showinitializer
	*/
	template <typename T>
	static inline std::function<std::string(const T&)> value_formatter{
		default_fmt_value<T>
	};

	/**
	@brief Formatter for <tt>@ref ECHO()</tt>'s message and source location
	@showinitializer
	*/
	static inline output::format_msg_src_t formatter {
		output::default_fmt_msg_src
	};
	///@} Configurable formatters

	/**
	@brief To satisfy <tt>@ref fgl::debug::output_formatter </tt>
	@returns the result of
		<tt>@ref formatter( @ref echo.string , @ref echo.location )</tt>
	*/
	static inline std::string format(const echo& echo)
	{ return formatter(echo.string, echo.location); }
};

static_assert(output_channel<output_config<echo>>);

///@cond FGL_INTERNAL_DOCS
namespace internal {
///@{ @name Internal Echo Senders

/// Constructs an echo object from a formatted message
[[maybe_unused]] static void emit_echo(
	std::string_view message,
	const std::source_location source = std::source_location::current())
{
	using config = fgl::debug::output_config<echo>;
	fgl::debug::output(
		echo{ config::message_formatter(message), source }
	);
}

/**
@brief Constructs an echo object from a formatted expression string
	concantinated with a formatted value
*/
[[maybe_unused]] static void emit_echov(
	const auto result,
	const std::string_view expression,
	const std::source_location source = std::source_location::current())
{
	using config = fgl::debug::output_config<echo>;
	std::string s{ config::expression_formatter(expression) };
	s += config::value_formatter<decltype(result)>(result);
	fgl::debug::output(echo{ std::move(s), source });
}

///@}
} // namespace internal
///@endcond INTERNAL

///@} group-debug-echo
}// namespace fgl::debug

#endif // ifndef FGL_DEBUG_ECHO_HPP_INCLUDED

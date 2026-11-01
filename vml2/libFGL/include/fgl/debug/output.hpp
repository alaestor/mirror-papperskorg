#pragma once
#ifndef FGL_DEBUG_OUTPUT_HPP_INCLUDED
#define FGL_DEBUG_OUTPUT_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cassert>
#include <stdexcept>
#include <string>
#include <string_view>
#include <iostream> // cout
#include <sstream> // ostringstream
#include <functional> // function, reference wrapper
#include <optional>
#include <source_location>

#include "../types/traits.hpp"
#include "../types/string_literal.hpp"

namespace fgl::debug {

/**
@file

@example example/fgl/debug/output_simple.cpp
	An example for @ref group-debug-output

@example example/fgl/debug/output_advanced.cpp
	An example for @ref group-debug-output

@defgroup group-debug-output Debug Output System

@brief libFGL debug output fascility

@details
	@parblock
	The <tt><fgl/debug/output.hpp></tt> header is for the configuration and
	access of the libFGL debug output stream. It establishes a configurable,
	type-specific output system and surrounding infrastructure. The output
	system is designed to be flexible and easy to use, though it can appear a
	bit daunting without a high-level overview.

	While intended as a "debug output system", the libFGL output system could
	be used for any purpose; including logging. For this reason, several
	output thresholds are provided in the form of
	<tt>@ref fgl::debug::priority</tt>. The output stream defaults to
	<tt>std::cout</tt> but can be redirected to any <tt>std::ostream</tt>.

	Output is sent on an <tt>@ref fgl::debug::output_channel</tt> which is a
	type that is used to identify and configure the channel. Each channel has
	a priority, enabled state, and name.

	The <tt>@ref fgl::debug::output</tt> class and its corresponding instance
	of the same name is used to configure the output system, as well as
	conditionally access the output stream. Access will only be granted if the
	channel <tt>@ref can_send()</tt>, at which point the type being sent to
	the output stream is formatted using a corresponding
	<tt>@ref fgl::debug::output_formatter</tt>; the returned value of which is
	sent to the output stream. A type which is both a channel and a formatter
	is called an <tt>@ref fgl::debug::output_handler</tt>.

	The <tt>@ref fgl::debug::output_config</tt> class template is a generic
	output handler. <tt>@ref fgl::debug::output::operator()()</tt>, the most
	easy-to-use way of sending to the debug output stream, relies on the
	existance of an <tt>output_config<T></tt> output handler. For this reason,
	and by convention, the <tt>output_config</tt> template is often specialized
	as an output handler for a specific type; often centralizing configuration
	options such as channel name, priority, formatters, etc.
	@endparblock

	@see The example programs @ref example/fgl/debug/output_simple.cpp and
		@ref example/fgl/debug/output_advanced.cpp

@{
*/

/**
@brief Output channel priority levels which are compared to
	<tt>@ref fgl::debug::output::priority_threshold</tt>
*/
enum class priority : unsigned char
{
	minimum,
	debug,
	info,
	message,
	event,
	warning,
	error,
	fatal,
	maximum
};

/**
@details
	An output channel is a type which is responsible for maintaining state
	and configuration options. It provides the channel's name, priority, and
	enabled state. Only the enabled state must be configurable thru the static
	<tt>@ref turn_on()</tt> and <tt>@ref turn_off()</tt> methods. Optionally, a
	channel may provide an interface to configure the other channel properties
	such as its name priority level; but such an interface isn't required.
	@note A channel's state must be static because it may be used across
		translation units.
@tparam T the type being evaluated
*/
template <class T>
concept output_channel = requires
{
	{ T::priority_level() } -> std::same_as<priority>;
	{ T::name() } -> std::same_as<std::string_view>;
	{ T::enabled() } -> std::same_as<bool>;
	{ T::turn_on() } -> std::same_as<void>;
	{ T::turn_off() } -> std::same_as<void>;
};

/**
@details
	A formatter is an object with a static member function named
	<tt>format</tt> which takes an object and returns a <tt>std::string</tt>
	representation of it.

@tparam T_formatter the type being evaluated
@tparam T_value the type which <tt>T_formatter</tt> is expected to format
*/
template <class T_formatter, typename T_value>
concept output_formatter = requires (const T_value& t)
{
	{ T_formatter::format(t) } -> std::same_as<std::string>;
};

/**
@brief The conjunction of an <tt>@ref fgl::debug::output_channel</tt> and an
	<tt>@ref fgl::debug::output_formatter</tt>
@details
	An output handler is the combination of an output channel and formatter.
	Convientient for centrilizing configuration and enables the use of
	<tt>fgl::debug::output(T_value)</tt> to easily send a formatted
	<tt>T_value</tt> object to the libFGL output stream.

@note By convention, output handlers are often implemented by defining a
	specialization of <tt>fgl::debug::output_config</tt>, which allows for the
	use of <tt>fgl::debug::output</tt>'s call operator.

@tparam T the type being evaluated
@tparam T_value the type which <tt>T</tt> is expected to format
*/
template <class T, typename T_value>
concept output_handler = output_channel<T> && output_formatter<T, T_value>;

/**
@brief Configurable <tt>output_channel</tt> implementation, provided for
	convenience.
@details An implementation of an output channel which provides public
	configuration "setters" for priority and channel name. As this is a very
	common implementation, it's provided here as an extensible type which is
	intended to be inherited.

@tparam T_enabled the initial enabled state of the channel
@tparam T_priority the initial priority of the channel
@tparam T_name the initial name of the channel
@tparam T_key a type used to differentiate between otherwise identical
	channel synthesizations.

@note to ensure that the correct synthesization is being referred to across
	compilation units, the <tt>T_name</tt> string literal should be defined as
	<tt>static inline constexpr fgl::string_literal</tt>

@internal @remarks Isn't this terrifying? I think this is terrifying and I
	wrote it. I think it's a horrible idea. Dear god! C++ clearly isn't
	designed to work this way... But it <i>does</i> work... And it's
	well-defined behavior as far as the language is concerned... And saves a
	bunch of typing... So I guess it stays! If you have a better idea, for the
	sake of my sanity, <b>please</b> let me know.
@endinternal
@todo should this be a macro?
*/
template
<
	bool T_enabled,
	priority T_priority,
	fgl::string_literal T_name,
	typename T_key = void
>
class simple_output_channel
{
	protected:
	///@{ @name channel properties
	static inline bool m_enabled{ T_enabled };
	static inline priority m_priority{ T_priority };
	static inline std::string m_name{ T_name };
	///@}

	public:

	/// enables the output channel
	static void turn_on() noexcept { m_enabled = true; }

	/// disables the output channel
	static void turn_off() noexcept { m_enabled = false; }

	/// @returns <tt>true</tt> if the channel is enabled
	[[nodiscard]] static bool enabled() noexcept { return m_enabled; }

	/// @returns the priority level of the channel
	[[nodiscard]] static priority priority_level() noexcept
	{ return m_priority; }

	/// @param priority The new priority level for the channel.
	static void priority_level(const priority priority) noexcept
	{ m_priority = priority; }

	/// @returns the name of the channel
	[[nodiscard]] static std::string_view name() noexcept { return m_name; }

	/// @param name The string to copy and be use as the new channel name.
	static void name(std::string_view name) { m_name = name; }
};

/**
@brief Global for disabling output from all
@ref fgl::debug::output_config "generic channels" @showinitializer
*/
static inline bool disable_generic_output_channels{ false };

///@cond INTERNAL
namespace internal {
static inline constexpr fgl::string_literal generic_cname{ "GENERIC" };
} // namespace internal
///@endcond

/**
@brief Generic output handler template
@details @parblock
		The generic definition of this template satisfies the
		<tt>output_handler</tt> concept and can be used to configure the
		generic channels and formatters on a per-type basis.

		@note If a type cannot use the generic <tt>output_config</tt>, a custom
			formatter and output channel must be defined. Ideally, they would
			be combined in a specialization of <tt>output_config<T></tt> which
			satisfies the <tt>fgl::debug::output_handler</tt> concept so that
			<tt>T</tt> may be used with <tt>fgl::debug::output(t)</tt>.

		By convention, this template is also specialized as the output
		configuration point for all libFGL types which interact with the
		libFGL output system. For example, an <tt>fgl::T</tt> might define a
		specialization of <tt>output_config<fgl::T></tt> which is can be used
		to turn on or off the <tt>fgl::T</tt> related output channel, or set a
		user-defined custom formatter for <tt>fgl::T</tt>.
	@endparblock

@note @parblock
	Output from generic type channels can be disabled by setting
	<tt>@ref fgl::debug::disable_generic_output_channels</tt> to <tt>true</tt>.

	By default, the channel names are <tt>"GENERIC"</tt> and the priority is
	<tt>fgl::debug::priority::info</tt>.
@endparblock

@attention @parblock
	For @ref default_formatter(const T&) "the generic default formatter" to
	function properly, <tt>T</tt> must satisfy
	<tt>@ref fgl::traits::printable<T_value></tt>. If it doesn't, the default
	formatter will throw a <tt>std::logic_error</tt> exception, whose
	<tt>what</tt> string will be an attention notice.

	Example notice:
	<blockquote>
		ATTENTION!
		Generated from: static std::string fgl::debug::output_config<T>::
		default_formatter(const T&) [with T = my_struct; std::string =
		std::__cxx11::basic_string<char>]

		The generic `output_config` formatter is only suitable
		for types which satisfy `fgl::traits::printable` by
		having an ostream `operator<<` overload. If you are
		seeing this, you should either define such an overload,
		or define a custom formatter for `T` and assign it to the
		static `fgl::debug::output_config<T>::format` member variable.
	</blockquote>
@endparblock

@tparam T the type which the <tt>output_config</tt> handles
*/
template <typename T>
class output_config
: public simple_output_channel
	<
		true,
		priority::info,
		internal::generic_cname,
		output_config<T>
	>
{
	output_config(auto&&...) = delete; ///< Should never be instantiated

	public:
	using channel_t = simple_output_channel
	<
		true,
		priority::info,
		internal::generic_cname,
		output_config<T>
	>;

	[[nodiscard]] static bool enabled() noexcept
	{ return channel_t::enabled() && !disable_generic_output_channels; }

	/**
	@brief Default formatter for <tt>T</tt>
	@throws std::logic_error @parblock
		if <tt>T</tt> doesn't satisfy <tt>fgl::traits::printable</tt>

		Refer to the "Attention" note in the detailed documtation for the
			generic <tt>@ref fgl::debug::output_config</tt>
		@endparblock
	*/
	[[nodiscard]] static std::string default_formatter(const T& t)
	{
		if constexpr (fgl::traits::printable<T>)
		{
			std::ostringstream oss;
			oss << t;
			return oss.str();
		}
		else
		{
			constexpr auto sl{ std::source_location::current() };
			std::string msg{ "\n\n\tATTENTION!\n\tGenerated from: " };
			msg += sl.function_name();
			msg +=
				"\n\n\tThe generic <tt>output_config</tt> formatter is only suitable"
				"\n\tfor types which satisfy <tt>fgl::traits::printable</tt> by"
				"\n\thaving an ostream <tt>operator<<</tt> overload. If you are"
				"\n\tseeing this, you should either define such an overload,"
				"\n\tor define a custom formatter for <tt>T</tt> and assign it to the"
				"\n\tstatic <tt>fgl::debug::output_config<T>::format</tt> member "
				" variable.\n\n"
			;
			throw std::logic_error(msg);
		}
	}

	/// Configurable formatter for <tt>T</tt>
	static inline std::function<std::string(const T&)> format{
		default_formatter
	};
};

static_assert(output_handler<output_config<int>, int>);

/// For access and configuration of the libFGL debug output stream
class output final
{
	public:
	/// Toggle for all libFGL debug output @showinitializer
	static inline bool enabled{ true };

	/**
	@brief The minimum output priority
	@details
		The priority of a channel must be greater than or equal to this
		threshold in order to send output to the output stream.
	@showinitializer
	*/
	static inline priority priority_threshold{ priority::minimum };

	/// A setter wrapper which prevents direct access to the output stream
	class output_stream_t
	{
		static inline std::ostream* m_output_stream{ &std::cout };
		friend class output;

		/// Getter @internal
		[[nodiscard]] std::ostream& operator()() const noexcept
		{ return *m_output_stream; }

		public:

		/// Writes uncommited changhes to the output stream
		output_stream_t& flush()
		{
			m_output_stream->flush();
			return *this;
		}

		/// Redirects the libFGL debug output stream.
		output_stream_t& operator=(std::ostream& output_stream) noexcept
		{
			m_output_stream = &output_stream;
			return *this;
		}
	};

	/**
	@brief The debug output stream (redirect and flush only)
	@details
		The output stream to which all libFGL debug output is sent. To prevent
		misuse of the output stream, this member prevents direct access. The
		stream may only be accessed thru other <tt>@ref fgl::debug::output</tt>
		methods which require an <tt>output_channel</tt> in order to send
		output.
		@note The default stream is <tt>std::cout</tt>
	*/
	static inline output_stream_t stream;

	///@{ @name Default Formatters

	/// The default formatter for channel name prefixes
	[[nodiscard]] static inline std::string default_fmt_head(
		const std::string_view name)
	{
		std::ostringstream ss;
		ss << '[' << name << "] ";
		return ss.str();
	}

	/// The formatter for messages
	[[nodiscard]] static inline std::string default_fmt_msg(
		const std::string_view message)
	{
		assert(not message.empty());
		std::ostringstream ss;
		ss
			<< "\n \\_____ " << message;
		return ss.str();
	}

	/// The default formatter for messages with a source location
	[[nodiscard]] static inline std::string default_fmt_msg_src(
		const std::string_view message,
		const std::source_location source)
	{
		std::ostringstream ss;
		ss
			<< "file:" << source.file_name()
			<< '(' << source.line() << ':' << source.column() << ") '"
			<< source.function_name()
			<< (message.empty() ? "" : "\n \\_____ ") << message;
		return ss.str();
	}
	///@} Default Formatters

	///@{ @name Formatter Configuration

	using format_head_t = std::function<std::string(std::string_view)>;

	using format_msg_t = std::function<std::string(std::string_view)>;

	using format_msg_src_t =
		std::function<std::string(std::string_view, std::source_location)>;

	/// Formatter for channel name prefixes @showinitializer
	static inline format_head_t format_head{ default_fmt_head };

	/// Formatter for messages @showinitializer
	static inline format_msg_t format_msg{ default_fmt_msg };

	/// Formatter for messages with a source location @showinitializer
	static inline format_msg_src_t format_msg_src{ default_fmt_msg_src };
	///@} Configurable Formatters

	/**
	@brief Checks whether or not a channel is permitted to send output
	@details Whether a channel may send output is determined by three factors:
		- whether or not <tt>fgl::debug::output::enabled</tt> is <tt>true</tt>
		- whether or not the channel's <tt>enabled()</tt> state is
			<tt>true</tt>
		- and whether or not the channel's <tt>@ref priority_level()</tt> is
			greater than or equal to the
			<tt>@ref fgl::debug::output::priority_threshold</tt>
	@tparam T_channel the channel being checked
	*/
	template <output_channel T_channel>
	static bool can_send() noexcept
	{
		return
			enabled
			&& T_channel::enabled()
			&& priority_threshold <= T_channel::priority_level();
	}

	///@{ @name Output Stream Accessor

	using optional_stream_t =
		std::optional<std::reference_wrapper<std::ostream>>;

	/**
	@brief Requests direct stream access for a given channel. Access is
		granted and the output stream is returned if the channel
		<tt>@ref can_send()<tt>
	*/
	template <output_channel T_channel>
	[[nodiscard]] static optional_stream_t channel_stream()
	{
		return (can_send<T_channel>())
			? std::make_optional(std::ref(stream()))
			: std::nullopt;
	}
	///@} Output Stream Accessor

	/**
	@{ @name Output Stream Senders
	@brief Attempts to format and send a message to the output stream from a
		given channel. If the channel <tt>@ref can_send()</tt>, the string
		representation of <tt>t</tt> returned from
		<tt>T_channel::format(t)</tt> is sent to the output stream, prefixed
		with the formatted channel name.
	*/

	/**
	@tparam T the type of the object being sent to the output stream
	@tparam T_channel the <tt>@ref fgl::debug::output_channel</tt> on which to
		send output
	@tparam T_formatter an <tt>@ref fgl::debug::output_formatter</tt> for
		<tt>T</tt> which will return the string representation of <tt>t</tt>
		which will be sent to the output stream
	@param[in] t the object being sent to the output stream
	*/
	template
	<
		typename T,
		fgl::debug::output_channel T_channel,
		fgl::debug::output_formatter<T> T_formatter
	>
	static void custom(const T& t)
	{
		if (const auto& os{ channel_stream<T_channel>() }; os)
		{
			os.value().get()
				<< format_head(T_channel::name())
				<< T_formatter::format(t)
				<< '\n';
		}
	}

	/**
	@tparam T the type of the object being sent to the output stream
	@tparam T_handler an <tt>@ref fgl::debug::output_handler</tt> for
		<tt>T</tt> which is used as both the
		<tt>@ref fgl::debug::output_channel</tt> and
		<tt>@ref fgl::debug::output_formatter</tt> for <tt>T</tt>
	@param[in] t the object being sent to the output stream
	*/
	template <typename T, output_handler<T> T_handler>
	static void handled(const T& t)
	{ custom<T, T_handler, T_handler>(t); }

	/**
	@tparam T the type of the object being sent to the output stream which
		must correspond with a specialization of
		<tt>@ref fgl::debug::output_config</tt>
		which satisfies the <tt>@ref fgl::debug::output_handler</tt> concept
	@param[in] t the object being sent to the output stream
	*/
	template <typename T>
	requires output_handler<output_config<T>, T>
	void operator()(const T& t) const
	{ handled<T, output_config<T>>(t); }

	///@} Output Stream Senders
};

/// For access and configuration of the libFGL debug output stream
[[maybe_unused]] static inline output output;

///@} end of group debug_output
} // namespace fgl::debug

#endif // FGL_DEBUG_OUTPUT_HPP_INCLUDED

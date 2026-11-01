#pragma once
#ifndef FGL_DEBUG_STOPWATCH_HPP_INCLUDED
#define FGL_DEBUG_STOPWATCH_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <chrono>
#include <string>
#include <string_view>
#include <functional> // function
#include <utility> // move
#include <algorithm> // sort
#include <numeric> // reduce
#include <sstream>
#include <ostream>
#include <iomanip> // setw
#include <vector>
#include <source_location>

#include "../environment/build_info.hpp"
#include "./constexpr_assert.hpp"
#include "./output.hpp"
#include "../types/traits.hpp"

namespace fgl::debug {

/**
@file

@example example/fgl/debug/stopwatch.cpp
	An example for @ref group-debug-stopwatch

@defgroup group-debug-stopwatch Stopwatch

@brief A model stopwatch and output formatting fascilities

@details
	The <tt><fgl/debug/stopwatch.hpp></tt> header provides a template for a
	generic stopwatch capable of recording elapsed time, a convenience alias
	for a <tt>std::chrono::steady_clock</tt> stopwatch, and relevant
	formatting fascilities which support sending output to libFGL's
	@ref group-debug-output.

	@see The example program @ref example/fgl/debug/stopwatch.cpp
@{
*/

///@cond FGL_INTERNAL_DOCS
namespace internal{
///@internal @returns [function name] + " in " + [file name]
inline std::string function_in_file(
	const std::source_location& sl = std::source_location::current())
{
	// Creates a default stopwatch name from the source location
	std::string default_name(sl.function_name());
	default_name += " in ";
	default_name += sl.file_name();
	return default_name;
}
}
///@endcond

/// Template for a generic model stopwatch.
template <fgl::traits::steady_clock T_clock = std::chrono::steady_clock>
class generic_stopwatch
{
public:
	/// The clock which the stopwatch is using
	using clock_t = T_clock;

	/// The type of time point the stopwatch's interface uses
	using time_point_t = std::chrono::time_point<clock_t>;

	/// The duration type of the clock
	using duration_t = T_clock::duration;

private:
	/// The possible states of a stopwatch.
	enum class state : unsigned char
	{
		reset,
		ticking,
		stopped
	};

	/**
	@brief The internal state of the stopwatch.
	@note This should not be used at all if <tt>NDEBUG</tt> is defined.
	*/
	state m_state;

	/// The previously recorded start time
	time_point_t m_last_point;

	/// Record of lap durations
	std::vector<duration_t> m_record;

public:
	/// The name of the stopwatch
	std::string name;

	/**
	@{ @name Constructors
	@param in_name The name of the stopwatch. If no name is provided, the
		name will be created from the source location.
	@param reserve The number of durations or laps that are expected to be
		recorded by the stopwatch. Defaults to <tt>1000</tt>. This is used by
		an internal vector to avoid potentially costly reallocations.
	@tparam T_clock A @ref fgl::traits::steady_clock to be used by the
		stopwatch. <tt>std::chrono::steady_clock</tt> by default.
	*/

	[[nodiscard]] constexpr explicit
	generic_stopwatch(
		std::string&& in_name = internal::function_in_file(),
		const std::size_t reserve = 1000)
	:
		m_state(state::reset),
		m_last_point{},
		m_record{},
		name(std::move(in_name))
	{ m_record.reserve(reserve); }

	[[nodiscard]] constexpr explicit
	generic_stopwatch(
		const std::string& in_name,
		const std::size_t reserve = 1000)
	:
		m_state(state::reset),
		m_last_point{},
		m_record{},
		name(in_name)
	{ m_record.reserve(reserve); }

	constexpr generic_stopwatch(const generic_stopwatch&) = default;
	constexpr generic_stopwatch(generic_stopwatch&&) noexcept = default;
	///@} Constructors

	/**
	@{ @name Stopwatch Recording Methods
	@brief These methods are used to start, lap, stop, and reset the stopwatch.
	@details
		Each pairing of <tt>start()</tt> and <tt>stop()</tt> calls will record
		the elapsed time between their time points as a lap duration. The
		<tt>lap()</tt> method is used to record a lap duration without
		stopping the stopwatch; its time point argument becomes the new start
		time.

		@note While these methods are <tt>constexpr</tt>, the default argument
			<tt>clock_t::now()</tt> may not be depending on the clock type. At
			the time of writing (C++20) there are no standard
			<tt>constexpr</tt> clocks.

		Stopwatches have an internal state to prevent undefined behavior,
		ensure correctness by enforcing a conceptually valid order of
		operations. Calling the methods <tt>@ref start()</tt>,
		<tt>@ref lap()</tt>, <tt>@ref stop()</tt>, and
		<tt>@ref stop_without_record()</tt> will assert that the stopwatch is
		in a valid state for that method's intended function. For example:
		the stopwatch cannot be stopped until after it has been started, and
		once it has been started it cannot be started again until t's been
		either stopped or reset. This is an artificial limitation which
		exists soley to prevent the accidental misuse which may lead to
		undefined behavior or invalid time records.

		@note To ensure optimal performance, the internal state mutations and
			assertions will be disabled and optimized out when <tt>NDEBUG</tt>
			is defined.

		@warning When the assertions are disabled by defining <tt>NDEBUG</tt>,
			it becomes possible to invoke undefined behavior if a method is
			called when the stopwatch is in a state which is invalid for that
			method! For example, calling <tt>@ref lap()</tt> or
			<tt>@ref stop()</tt> when the stopwatch is already stopped, or
			calling <tt>@ref start()</tt> when it has already been started.
	*/

	/**
	@brief Records the <tt>time_point</tt> as the start time.
	@param time_point The time point to record as the start time.
	@note The stopwatch must not already be "ticking". Starting a stopwatch is
		only valid after it's been stopped, reset, or first initialized.
	*/
	constexpr void start(const time_point_t time_point = clock_t::now())
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_state != state::ticking);
		if constexpr (fgl::debug_build)
			m_state = state::ticking;
		m_last_point = time_point;
	}

	/**
	@brief Without stopping the watch, records a lap whose duration is
		<tt>time_point</tt> subtracted from the start time, and then updates
		the start time.
	@param time_point The time point which will be subtracted from the start
		time to produce a lap duration. This time point will become the new
		start time after the lap duration has been recorded.
	@note The stopwatch must be "ticking". Lapping a stopwatch is only valid
		after it has been started.
	@remarks For high precision timing, it's recommended to
		<tt>@ref stop()</tt> and then <tt>@ref start()</tt> the stopwatch
		rather than calling <tt>@ref lap()</tt> as the time between emplacing
		the lap duration and updating the start time may be significant,
		especially if the number of laps grows beyond what was reserved when
		the stopwatch was constructed.
	*/
	constexpr void lap(const time_point_t time_point = clock_t::now())
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_state == state::ticking);
		FGL_DEBUG_CONSTEXPR_ASSERT(time_point >= m_last_point);
		m_record.emplace_back(time_point - m_last_point);
		m_last_point = time_point;
	}

	/**
	@brief Records a lap whose duration is <tt>time_point</tt> subtracted from
		the start time, and then stops the stopwatch.
	@param time_point The time point which will be subtracted from the start
		time to produce a lap duration.
	@note The stopwatch must be "ticking". Stopping a stopwatch is only valid
		after it has been started.
	*/
	constexpr void stop(const time_point_t time_point = clock_t::now())
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_state == state::ticking);
		FGL_DEBUG_CONSTEXPR_ASSERT(time_point >= m_last_point);
		if constexpr (fgl::debug_build)
			m_state = state::stopped;
		m_record.emplace_back(time_point - m_last_point);
	}

	/**
	@brief Stops the stopwatch without recording a lap duration.
	@remarks This is particularly useful to discard a potential lap, or when
		the stopwatch is being used to record laps but should not record the
		time between the last lap and stopping the stopwatch.
	@note The stopwatch must be "ticking". Stopping a stopwatch is only valid
		after it has been started.
	*/
	constexpr void stop_without_record() noexcept
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_state == state::ticking);
		if constexpr (fgl::debug_build)
			m_state = state::stopped;
	}

	/**
	@brief Resets the stopwatch to its initial state, clearing all recorded
		durations.
	@note This method is valid in any state.
	*/
	constexpr void reset()
	{
		m_record.clear();
		if constexpr (fgl::debug_build)
			m_state = state::reset;
	}
	///@}  Stopwatch Recording Methods

	/// @returns The number of recorded laps
	[[nodiscard]] constexpr std::size_t number_of_laps() const
	{ return m_record.size(); }

	/**
	@note The stopwatch must be in a "stopped" state.
	@param lap_number The lap to return the duration of. Must be less than the
		number of recorded laps.
	@returns The duration of the specified lap.
	*/
	[[nodiscard]] constexpr
	duration_t get_lap(const std::size_t lap_number) const
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_state == state::stopped);
		return m_record.at(lap_number);
	}

	/**
	@returns The duration of the most recent lap.
	@note The stopwatch must be in a "stopped" state (which also means it
		must have at least one lap recorded).
	*/
	[[nodiscard]] constexpr duration_t previous_lap() const
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_state == state::stopped);
		return m_record.back();
	}

	/**
	@returns A <tt>const</tt> reference to the internal record of lap
		durations.
	@note The stopwatch must be in a "stopped" state.
	*/
	[[nodiscard]] constexpr
	const std::vector<duration_t>& get_all_laps() const
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_state == state::stopped);
		return m_record;
	}

	/**
	@param start_lap The lap to start the elapsed time summation from.
		Must be less than the number of recorded laps.
	@param end_lap The lap to end the elapsed time summation at.
		Must be less than or equal to the the number of recorded laps, and
		greater than <tt>start_lap</tt>.
	@returns The summation of durations
		[<tt>start_lap</tt>, <tt>end_lap</tt>).
	@note The stopwatch must be in either a "ticking" or "stopped" state.
	*/
	[[nodiscard]] constexpr duration_t elapsed_between_laps(
		const std::size_t start_lap,
		const std::size_t end_lap
	) const
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_state > state::reset);
		FGL_DEBUG_CONSTEXPR_ASSERT(start_lap < end_lap);
		FGL_DEBUG_CONSTEXPR_ASSERT(end_lap <= m_record.size());
		using diff_t = decltype(m_record)::difference_type;
		return std::reduce(
			m_record.cbegin() + static_cast<diff_t>(start_lap),
			m_record.cbegin() + static_cast<diff_t>(end_lap),
			duration_t{}
		);
	}

	/**
	@returns The sum of all recorded lap durations.
	@details Equivalent to calling
		<tt>@ref elapsed_between_laps(0, number_of_laps())</tt>.
	@note The stopwatch must be in either a "ticking" or "stopped" state.
	@note The elapsed time is **NOT** the last stop time point subtracted from
		the first start time point. It is the sum of all recorded laps. There
		may be time between <tt>@ref start()<tt>s and <tt>@ref stop()<tt>s
		which isn't represented; this is an intentional design choice.
	*/
	[[nodiscard]] constexpr duration_t elapsed() const
	{ return elapsed_between_laps(0, m_record.size()); }

	struct statistics
	{
		std::size_t number_of_laps;
		duration_t total_elapsed;
		duration_t mean;
		duration_t median;
		duration_t min;
		duration_t max;

		/**
		@{ @name Constructors
		@note To generate statistics for a stopwatch, use the the stopwatch's
			<tt>@ref calculate_statistics()</tt> factory method.
		@param[in] sorted_laps A vector of lap durations, sorted in ascending
			order, which will be used to produce the statistics.
		*/
		[[nodiscard]] explicit constexpr
		statistics(const std::vector<duration_t>& sorted_laps) noexcept
		:
			number_of_laps{ sorted_laps.size() },
			total_elapsed{
				std::reduce(
					sorted_laps.cbegin(),
					sorted_laps.cend(),
					duration_t{}
				)
			},
			mean{ get_mean(sorted_laps) },
			median{ get_median(sorted_laps) },
			min{ sorted_laps.front() },
			max{ sorted_laps.back() }
		{
			FGL_DEBUG_CONSTEXPR_ASSERT(
				std::is_sorted(sorted_laps.cbegin(), sorted_laps.cend())
			);
		}

		[[nodiscard]] explicit constexpr statistics(const statistics&)
			noexcept = default;
		///@} Constructors

		private:

		/// @returns The mean of a sorted vector of durations
		[[nodiscard]] static constexpr
		duration_t get_mean(const std::vector<duration_t>& v) noexcept
		{
			const auto size{ static_cast<duration_t::rep>(v.size()) };

			return (size != 0)
				? std::reduce(v.cbegin(), v.cend(), duration_t{}) / size
				: duration_t{};
		}

		/// @returns The median from a sorted vector of durations
		[[nodiscard]] static constexpr
		duration_t get_median(const std::vector<duration_t>& v) noexcept
		{
			using rep_t = typename duration_t::rep;
			const std::size_t size{ v.size() };
			switch (size)
			{
			break; case 0: return {};
			break; case 1: return { v[0] };
			break; case 2: return { (v[0] + v[1]) / rep_t{2} };
			break; default:
				const std::size_t mid{ size / rep_t{2} };
				return {
					(size % 2 == 0)
					? (v[mid-1] + v[mid]) / rep_t{2}
					: v[mid]
				};
			}
		}
	};

	/**
	@returns A <tt>@ref statistics</tt> object containing the statistics of
		the recorded lap durations.
	@note The stopwatch must be in a "stopped" state.
	*/
	[[nodiscard]] constexpr statistics calculate_statistics() const
	{
		FGL_DEBUG_CONSTEXPR_ASSERT(m_state == state::stopped);
		std::vector<duration_t> v(m_record);
		std::sort(v.begin(), v.end());
		return statistics(v);
	}
};

/// A convenient alias, as this is by far the most common use case.
using stopwatch = generic_stopwatch<std::chrono::steady_clock>;

/// Disables all stopwatch output channels if set to <tt>true</tt>
static inline bool disable_stopwatch_output_channels{ false };

///@internal
namespace internal {
static inline constexpr fgl::string_literal stopwatch_cname{ "STOPWATCH" };
} // namespace internal

/**
@brief A template for a template specializion of output handler for using
	stopwatches with libFGL's @ref group-debug-output.

@note The channel state is specific to each synthesizations of the
	stopwatch-specialized <tt>output_config</tt> template. To disable output
	from all synthesized stopwatch channels, set
	<tt>fgl::debug::disable_stopwatch_output_channels</tt> to <tt>true</tt>

@see @ref group-debug-output and @ref fgl::debug::output::operator()()
@tparam T_clock The clock type to use for the stopwatch.

@todo formatters could do with a rework, and the channel formatter could be
	made configurable.

@todo should be able to print an empty stopwatch
*/
template <fgl::traits::steady_clock T_clock>
class output_config<generic_stopwatch<T_clock>>
: public simple_output_channel
	<
		true,
		priority::info,
		internal::stopwatch_cname,
		output_config<generic_stopwatch<T_clock>>
	>
{
	output_config(auto&&...) = delete; ///< should never be instantiated
	public:
	using channel_t = simple_output_channel
	<
		true,
		priority::info,
		internal::stopwatch_cname,
		output_config<generic_stopwatch<T_clock>>
	>;

	using stopwatch_t = generic_stopwatch<T_clock>;

	[[nodiscard]] static constexpr bool is_enabled() noexcept
	{ return channel_t::is_enabled() && !disable_stopwatch_output_channels; }

	/**
	@brief Stopwatch formatter method to satisfy
		<tt>fgl::debug::output_formatter</tt>
	*/
	[[nodiscard]] static std::string format(const stopwatch_t& sw)
	{
		std::string temp("Statistics: ");
		const auto& stats{ statistics_formatter(sw.calculate_statistics())};
		temp.reserve(temp.size() + sw.name.size() + stats.size() + 1);
		temp += sw.name;
		temp += '\n';
		temp += stats;
		return output::default_fmt_msg(temp);
	}

	///@{ @name Default Formatters

	[[nodiscard]] static
	std::string default_duration_formatter(stopwatch_t::duration_t duration)
	{
		std::stringstream ss;
		const auto process{
			[&duration, &ss](const auto unit_time) -> void
			{
				if (unit_time.count() != 0)
				{
					duration -= unit_time;
					ss << unit_time << ' ';
				}
			}
		};
		using namespace std::chrono;
		process(duration_cast<years>(duration));
		process(duration_cast<months>(duration));
		process(duration_cast<weeks>(duration));
		process(duration_cast<days>(duration));
		process(duration_cast<hours>(duration));
		process(duration_cast<minutes>(duration));
		process(duration_cast<seconds>(duration));
		process(duration_cast<milliseconds>(duration));
		process(duration_cast<microseconds>(duration));
		process(duration_cast<nanoseconds>(duration));
		assert(duration.count() == 0);
		return ss.str();
	}

	[[nodiscard]] static std::string default_statistics_formatter(
		const stopwatch_t::statistics& stats)
	{
		std::stringstream ss;
		ss
			<< "\tNumber of laps: " << stats.number_of_laps
			<< "\n\tTotal elapsed:  " << duration_formatter(stats.total_elapsed)
			<< "\n\tMean lap:       " << duration_formatter(stats.mean)
			<< "\n\tMedian lap:     " << duration_formatter(stats.median)
			<< "\n\tMin lap:        " << duration_formatter(stats.min)
			<< "\n\tMax lap:        " << duration_formatter(stats.max);
		return ss.str();
	}

	[[nodiscard]] static
	std::string default_stopwatch_formatter(const stopwatch_t& sw)
	{
		std::stringstream ss;
		const std::size_t n{ sw.number_of_laps() };
		ss
			<< sw.name << ": " << duration_formatter(sw.elapsed())
			<< " (" << n << " lap" << (n == 1 ? "" : "s") << ")";
		return ss.str();
	}
	///@} Default Formatters

	///@{ @name Configurable formatters

	using duration_formatter_t =
		std::function<std::string(typename stopwatch_t::duration_t)>;

	using statistics_formatter_t =
		std::function<std::string(const typename stopwatch_t::statistics&)>;

	using stopwatch_formatter_t =
		std::function<std::string(const stopwatch_t&)>;

	/// @showinitializer
	static inline duration_formatter_t duration_formatter{
		default_duration_formatter
	};

	/// @showinitializer
	static inline statistics_formatter_t statistics_formatter{
		default_statistics_formatter
	};

	/// @showinitializer
	static inline stopwatch_formatter_t stopwatch_formatter{
		default_stopwatch_formatter
	};

	///@} Configurable formatters
};

static_assert(output_handler<output_config<stopwatch>, stopwatch>);

/**
@brief <tt>std::ostream</tt> support for stopwatches. Utilizes the
	<tt>@ref stopwatch_formatter</tt> member
*/
template <fgl::traits::steady_clock T_clock>
std::ostream& operator<<(
	std::ostream& os,
	const generic_stopwatch<T_clock>& sw)
{
	using config = output_config<generic_stopwatch<T_clock>>;
	return os << config::stopwatch_formatter(sw);
}

/**
@{ @name Loose Helper Functions
@brief String formatting functions which make use of the
	<tt>@ref output_config</tt> stopwatch specialization template's
	configurable formatters.
*/

template <typename T_clock>
[[nodiscard]] inline std::string to_string_duration(
	const typename generic_stopwatch<T_clock>::duration_t duration)
{
	return output_config<generic_stopwatch<T_clock>>::duration_formatter(
		duration
	);
}

template <fgl::traits::steady_clock T_clock>
[[nodiscard]] inline std::string to_string_minimal(
	const generic_stopwatch<T_clock>& sw)
{
	return output_config<generic_stopwatch<T_clock>>::stopwatch_formatter(sw);
}

template <fgl::traits::steady_clock T_clock>
[[nodiscard]] inline std::string to_string_statistics(
	const generic_stopwatch<T_clock>& sw)
{
	std::string s(sw.name);
	s += "\n \\_____ Statistics\n";
	s += output_config<generic_stopwatch<T_clock>>::statistics_formatter(
		sw.calculate_statistics()
	);
	return s;
}

template <fgl::traits::steady_clock T_clock>
[[nodiscard]] inline std::string to_string_full(
	const generic_stopwatch<T_clock>& sw)
{
	std::stringstream ss;
	ss << to_string_statistics(sw);
	if (const std::size_t nlaps{ sw.number_of_laps() }; nlaps > 0)
	{
		// number of digits used to represent the highest lap number
		const int max_digits{
			[](std::size_t value) -> int
			{ // not size_t because std::setw(int)
				int number_columns{ 1 };
				for (; value > 9; value /= 10, ++number_columns);
				return number_columns;
			}(nlaps - 1) // laps count from 0
		};

		ss << "     \\_ Lap durations\n";
		for (std::size_t i{}; const auto lap : sw.get_all_laps())
		{
			ss
				<< "\tLap " << std::setw(max_digits) << i++ << ": "
				<< output_config<generic_stopwatch<T_clock>>::duration_formatter(
					lap)
				<< '\n';
		}
	}
	return ss.str();
}

///@} Loose Helper Functions

} // namespace fgl::debug

#endif // FGL_DEBUG_STOPWATCH_HPP_INCLUDED

#pragma once
#ifndef FGL_UTILITY_SLEEP_HPP_INCLUDED
#define FGL_UTILITY_SLEEP_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <chrono>
#include <thread>

#include "../types/traits.hpp"

namespace fgl {

/**
@file

@defgroup group-utility-sleep Sleep

@brief High-precision sleep functions

@details The only guarentee that can be provided by these functions is that that
	the functions will return some time after requested duration. The exact
	delays are subject to the whims of the scheduler, platform, and
	<tt>std::chrono::steady_clock</tt> implementation. This kind of
	"low-bound guarentee" is rather typical for sleep delay functions, as more
	narrow guarentees aren't possible without operating system level support,
	such as those in real-time execution environments.

	@note while the function templates are <tt>constexpr</tt> specified, the
		synthesizations will not valid compile-time function unless the static
		<tt>T_clock::now()</tt> method is <tt>constexpr</tt>.

	@remarks  @parblock
		Prefer the use of <tt>std::this_thread::sleep_for</tt> and
		<tt>std::this_thread::yield</tt> in situations where where they are
		more appropriate, such as timescales which are longer than a
		millisecond.

		For accuracy over longer time scales, the standard provided sleep
		functions can be layered on top of these functions. For example,
		<tt>sleep_until</tt> or <tt>sleep_for</tt> can be used to bring the
		required resolution within the appropriate operation timescale for
		<tt>micro_sleep</tt>, which can be used to further reduce the
		timescale until <tt>nano_sleep</tt> should take over. With some
		testing to figure out the ranges of the ideal timeframes, this
		technique can feisably provide very good sleep accuracy on certain
		platforms and environments. However this approach isn't a magic bullet
		due to inherent inconsistencies and limitations of sub-time-slice
		delays.
	@endparblock

@todo create a steady <tt>constexpr</tt> clock and rework unit test
@{
*/

/**
@brief A yielding loop which continues until at least @p until_time has been
	reached.
@details Implemented as a microsecond resolution <tt>yield</tt>ing loop which
	may be used for sub-<tt>sleep_for</tt> timing; <tt>+~3μs</tt> under ideal
	conditions in most environments.
@tparam T_clock the clock to use for the timing.
@param until_time the minimum time to sleep until.
*/
template <fgl::traits::steady_clock T_clock>
constexpr inline void micro_sleep(
	const typename T_clock::time_point until_time)
noexcept(noexcept(T_clock::now()))
{ while (T_clock::now() < until_time) std::this_thread::yield(); }

/**
@brief Equivalent to calling
	<tt>@ref fgl::micro_sleep<T_clock>(T_clock::now() + for_duration);</tt>
@tparam T_clock the clock to use for the timing.
@param for_duration the minimum duration to sleep for.
*/
template <fgl::traits::steady_clock T_clock = std::chrono::steady_clock>
constexpr inline  void micro_sleep(
	const std::chrono::microseconds for_duration)
noexcept(noexcept(T_clock::now()))
{ micro_sleep<T_clock>(T_clock::now() + for_duration); }

/**
@brief A non-yielding spinloop which continues until at least @p until_time
	has been reached.
@details Implemented as a nanosecond resolution hot loop which may be used for
	sub-<tt>yield</tt> timing; <tt>+~400ns</tt> under ideal conditions in
	most environments.
@tparam T_clock the clock to use for the timing.
@param until_time the minimum time to sleep until.
*/
template <fgl::traits::steady_clock T_clock>
constexpr inline void nano_sleep(
	const typename T_clock::time_point until_time)
noexcept(noexcept(T_clock::now()))
{ while (T_clock::now() < until_time); }

/**
@brief Equivalent to calling
	<tt>@ref fgl::nano_sleep<T_clock>(T_clock::now() + for_duration);</tt>
@tparam T_clock the clock to use for the timing.
@param for_duration the minimum duration to sleep for.
*/
template <fgl::traits::steady_clock T_clock = std::chrono::steady_clock>
constexpr inline void nano_sleep(
	const std::chrono::nanoseconds for_duration)
noexcept(noexcept(T_clock::now()))
{ nano_sleep<T_clock>(T_clock::now() + for_duration); }

///@} group-utility-sleep

} // namespace fgl

#endif // FGL_UTILITY_SLEEP_HPP_INCLUDED

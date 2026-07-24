#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <array>
#include <ranges> // subrange

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/debug/stopwatch.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

/// long term TODO: periodically check if this is still needed
// gcc's problem: experimental implementation of constexpr std::vector
// warning doesnt occur when executing at runtime...
#pragma GCC diagnostic ignored "-Wstrict-overflow"

using namespace std::chrono_literals;
using namespace std::chrono;
using fgl::debug::stopwatch;

// should use more datasets or a pseudo-random simulated clock. Meh. Hardcoded.
static constexpr std::array passage_of_time{ 2ns, 46ns, 80ns, 82ns, 59ns, 65ns, 13ns, 90ns, 71ns, 96ns, 78ns, 55ns, 98ns, 60ns, 84ns, 57ns, 4ns, 11ns, 64ns, 43ns, 45ns, 61ns, 14ns, 63ns, 1ns, 51ns, 68ns, 47ns, 8ns, 87ns, 93ns, 7ns, 53ns, 48ns, 41ns, 81ns, 36ns, 5ns, 76ns, 6ns, 85ns, 69ns, 70ns, 9ns, 97ns, 38ns, 95ns, 66ns, 58ns, 56ns, 92ns, 72ns, 75ns, 42ns, 62ns, 3ns, 83ns, 77ns, 88ns, 12ns, 100ns, 86ns, 10ns, 49ns, 74ns, 37ns, 54ns, 94ns, 99ns, 35ns, 73ns, 89ns, 39ns, 91ns, 67ns, 50ns, 40ns, 44ns, 52ns, 79ns };
// 2, 46, 80, 82, 59, 65, 13, 90, 71, 96, 78, 55, 98, 60, 84, 57, 4, 11, 64, 43, 45, 61, 14, 63, 1, 51, 68, 47, 8, 87, 93, 7, 53, 48, 41, 81, 36, 5, 76, 6, 85, 69, 70, 9, 97, 38, 95, 66, 58, 56, 92, 72, 75, 42, 62, 3, 83, 77, 88, 12, 100, 86, 10, 49, 74, 37, 54, 94, 99, 35, 73, 89, 39, 91, 67, 50, 40, 44, 52, 79,
// statistics: mean 57, median 60.5, min 1, max 100, range 99, sum 4560, size 80, stddev 28.85, variance 832.40, rel stddev 50.616%, CoV 0.50616, mean abs dev 23.65

static constexpr std::array time_points{
	[]()
	{
		std::array<stopwatch::time_point_t, passage_of_time.size()> points{};
		stopwatch::time_point_t t{};
		for (std::size_t i{}; i < passage_of_time.size(); ++i)
		{
			t += passage_of_time[i];
			points[i] = t;
		}
		return points;
	}()
};

static constexpr std::array durations{
	[]()
	{
		std::array<stopwatch::duration_t, time_points.size()-1> temp{};
		for (std::size_t i{}; i < time_points.size()-1; ++i)
			temp[i] = time_points[i+1] - time_points[i];
		return temp;
	}()
};

constexpr stopwatch create_simulated_stopwatch()
{
	stopwatch sw("tester");
	sw.start(time_points.front());
	using std::ranges::subrange;
	for (const auto p : subrange(time_points.begin()+1, time_points.end()-1))
		sw.lap(p);
	sw.stop(time_points.back());
	return sw;
}

constexpr bool test_statistics(const stopwatch::statistics stats)
{
	constexpr_assert(stats.number_of_laps == durations.size());
	constexpr_assert(stats.mean == 57ns);
	constexpr_assert(stats.median == 61ns);
	constexpr_assert(stats.min == 1ns);
	constexpr_assert(stats.max == 100ns);
	constexpr_assert(
		stats.total_elapsed == time_points.back() - time_points.front()
	);
	return true;
}

constexpr bool test_stopwatch()
{
	const stopwatch sw{ create_simulated_stopwatch() };
	constexpr_assert(sw.number_of_laps() == durations.size());
	constexpr_assert(sw.get_lap(0) == durations.front());
	constexpr_assert(sw.previous_lap() == durations.back());
	{
		const auto laps = sw.get_all_laps();
		for (unsigned int i{}; i < durations.size(); ++i)
			constexpr_assert(laps[i] == durations[i]);
	}
	constexpr_assert(sw.elapsed() == time_points.back() - time_points.front());
	constexpr_assert(
		sw.elapsed_between_laps(0, sw.number_of_laps()) == sw.elapsed()
	);
	constexpr_assert(test_statistics(sw.calculate_statistics()));
	return true;
}

int main()
{
	static_assert(test_stopwatch()); // also tests stopwatch::statistics
	return EXIT_SUCCESS;
}

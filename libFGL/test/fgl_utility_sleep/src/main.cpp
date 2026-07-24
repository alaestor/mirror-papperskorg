#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <cassert>

#define FGL_SHORT_MACROS
#include <fgl/debug/stopwatch.hpp>

#include <fgl/utility/sleep.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

using namespace std::chrono;
using fgl::debug::stopwatch;

constexpr unsigned int test_iterations{ 100 };

bool test_nano_sleep()
{
	stopwatch sw;
	constexpr auto target{ nanoseconds(100) };
	for (unsigned int i{}; i < test_iterations; ++i)
	{
		sw.start();
		fgl::nano_sleep(target);
		sw.stop();
		std::this_thread::yield();
	}
	const auto stats{ sw.calculate_statistics() };
	assert(stats.total_elapsed >= target * test_iterations);
	assert(stats.median < target * 3);
	return true;
}

bool test_micro_sleep()
{
	stopwatch sw;
	constexpr auto target{ microseconds(1) };
	for (unsigned int i{}; i < test_iterations; ++i)
	{
		sw.start();
		fgl::micro_sleep(target);
		sw.stop();
		std::this_thread::yield();
	}
	const auto stats{ sw.calculate_statistics() };
	assert(stats.total_elapsed >= target * test_iterations);
	assert(stats.median < target * 3);
	return true;
}


int main()
{
	assert(test_nano_sleep());
	assert(test_micro_sleep());
	return EXIT_SUCCESS;
}

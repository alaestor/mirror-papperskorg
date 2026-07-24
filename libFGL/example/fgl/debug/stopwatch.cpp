/**
This file is an example for <fgl/debug/stopwatch.hpp>

--- Example output
-------------------------------------------------------------------------------
cout: One inner yield: 10µs 300ns  (100 laps)

[STOPWATCH]
 \_____ Statistics: One inner yield
        Number of laps: 100
        Total elapsed:  10µs 300ns
        Mean lap:       103ns
        Median lap:     100ns
        Min lap:        100ns
        Max lap:        200ns
[STOPWATCH]
 \_____ Statistics: One inner yield, one check yield, and increment
        Number of laps: 100
        Total elapsed:  17µs 900ns
        Mean lap:       179ns
        Median lap:     200ns
        Min lap:        100ns
        Max lap:        300ns
*/

#include <iostream>
#include <thread>

#define NDEBUG // eliminate some stopwatch assertions
#include <fgl/debug/stopwatch.hpp>

int main()
{
	fgl::debug::stopwatch sw("One inner yield");
	std::this_thread::yield(); // warmup

	// here, we're aiming to only time the inner yield
	for (int i{}; std::this_thread::yield(), i < 100; ++i)
	{
		sw.start();
		std::this_thread::yield();
		sw.stop();
	}

	// send the stopwatch to the `cout` stream using the `operator<<` overload
	std::cout << "cout: "<< sw << '\n' << std::endl;

	// send the stopwatch to libFGL's debug output stream; using the
	// `output_config<stopwatch>` default formatter which includes statistics
	fgl::debug::output(sw);

	// reset the stopwatch, clearing all recorded laps
	sw.reset();

	// change the name of the stopwatch
	sw.name = "One inner yield, one check yield, and increment";

	// here, we're also timing the condition check and increment statements
	sw.start();
	for (int i{}; std::this_thread::yield(), i < 100; ++i)
	{
		std::this_thread::yield();
		sw.lap();
	}
	// regular stop would record a lap, so we discard it.
	sw.stop_without_record();

	fgl::debug::output(sw);

	// flush because the program terminates right after this
	fgl::debug::output::stream.flush();
}

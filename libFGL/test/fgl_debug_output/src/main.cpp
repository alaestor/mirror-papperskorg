#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <cassert>
#include <sstream>
#include <string>

#include <fgl/debug/output.hpp>

#include "notmain.hpp"

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

/// TODO channel & threshold tests

struct my_struct
{
	int x, y, z;
};

template <>
class fgl::debug::output_config<my_struct>
{
	static inline bool m_enabled{ true };
	public:
	output_config(auto&&...) = delete;
	~output_config() = delete;
	static void turn_on() { m_enabled = true; }
	static void turn_off() { m_enabled = false; }
	static bool enabled() { return m_enabled; }
	static fgl::debug::priority priority_level() { return priority::info; }
	static std::string_view name() { return "my struct"; }

	static std::string format(const my_struct& o)
	{
		std::ostringstream ss;
		ss << o.x << " " << o.y << " " << o.z;
		return ss.str();
	}
};

static_assert(
	fgl::debug::output_handler<
		fgl::debug::output_config<my_struct>, my_struct
	>
);

using test_config = fgl::debug::output_config<my_struct>;

/// GLOBAL OUTPUT TEST STREAM
std::stringstream sstream;

std::string last_output()
{
	const std::string s{ sstream.str() };
	sstream.clear(); // reset sstream state
	sstream.str(""); // empty stream
	return s;
}

using namespace fgl::debug;

bool test_output_channel_enable_status()
{
	// test channel enable status
	test_config::turn_off();
	output(my_struct{1,2,3});
	assert(last_output().empty());

	output('a');
	assert(!last_output().empty());
	output_config<char>::turn_off();
	output('a');
	assert(last_output().empty());
	output_config<char>::turn_on();
	output('a');
	assert(!last_output().empty());

	test_config::turn_on();
	output(my_struct{1,2,3});
	assert(!last_output().empty());

	// global generic output switch
	disable_generic_output_channels = true;
	output('a');
	assert(last_output().empty());
	disable_generic_output_channels = false;
	return true;
}

bool test_priority_threshold()
{
	output::priority_threshold = priority::warning;
	assert(output::priority_threshold == priority::warning);
	assert(output::priority_threshold > priority::info);
	assert(test_config::priority_level() == priority::info);
	assert(fgl::debug::output::can_send<test_config>() == false);
	output(my_struct{1,2,3});
	assert(last_output().empty());
	return true;
}

int main()
{
	assert(test_config::format({ 1, 2, 3 }) == std::string("1 2 3"));
	fgl::debug::output::priority_threshold = fgl::debug::priority::minimum;

	// test basic stream functionality & across translation units
	output::stream = sstream;
	const std::string maintest("main test");
	output(maintest);
	assert(
		last_output() ==
			output::format_head(output_config<decltype(maintest)>::name())
			+ maintest + '\n'
	);

	notmain();
	assert(
		last_output() ==
			output::format_head(output_config<const char[13]>::name())
			+ "notmain test\n"
	);

	// test generic output
	output("hello, world");
	assert(
		last_output() ==
		output::format_head(output_config<const char[13]>::name())
			+ "hello, world\n"
	);

	// test specific output
	fgl::debug::output(my_struct{3,4,5});
	assert(
		last_output() == output::format_head(test_config::name())
			+ "3 4 5\n"
	);

	assert(test_output_channel_enable_status());
	assert(test_priority_threshold());

	return EXIT_SUCCESS;
}

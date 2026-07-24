#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE, byte
#include <cassert>
//#include <utility> // std::unreachable

#define FGL_SHORT_MACROS

#include <fgl/debug/exception_occurs.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

enum class throw_command
{
	nothing,
	throw_int,
	throw_float,
};

using enum throw_command;

void f(const throw_command throw_command)
{
	switch (throw_command)
	{
	break; case nothing: return;
	break; case throw_int: throw int{};
	break; case throw_float: throw float{};
	break; default: assert(false); //TODO C++23 std::unreachable();
	};
}

bool test_exception_occurs() try
{
	assert(!exception_occurs(f(nothing)));
	assert(exception_occurs(f(throw_int)));
	assert(exception_occurs(f(throw_float)));
	//*/
	return true;
}
catch (...)
{ return false; } // an exception leaked?

bool test_specific_exception_occurs() try
{
	assert(!specific_exception_occurs(int, f(nothing)));
	assert(!specific_exception_occurs(float, f(nothing)));
	assert(specific_exception_occurs(int, f(throw_int)));
	assert(specific_exception_occurs(float, f(throw_float)));

	assert(
		specific_exception_occurs(
			float, // only catching int, float exception should propagate
			specific_exception_occurs(int, f(throw_float))
		)
	);

	assert(
		specific_exception_occurs(
			int, // only catching float, int exception should propagate
			specific_exception_occurs(float, f(throw_int))
		)
	);

	return true;
}
catch (...)
{ return false; } // an exception leaked?

int main()
{
	assert(test_exception_occurs());
	assert(test_specific_exception_occurs());
	return EXIT_SUCCESS;
}

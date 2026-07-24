/**
This file is an example for <fgl/debug/exception_occurs.hpp>

--- Example output: none; the program compiles and exits successfully
*/
#include <stdexcept>
#include <cassert>

#define FGL_SHORT_MACROS
#include <fgl/debug/exception_occurs.hpp>

// whoever wrote this must suck
void f_always_throws_runtime_error()
{ throw std::runtime_error("I'm a terrible function"); }

// whoever wrote this is a genius
void f_never_throws() noexcept
{}

// a fictitious stupid library, probably written by the guy who sucks
namespace stupid_lib
{
struct busy{}; // an exception that is thrown when stupidLib is being stupid

/***************************
 ***   BEHOLD, A THING   ***
 ***************************/
class Thing
{
	friend Thing make_thing();
	static inline int failures{ 0 };

	// Thing can't be valid unless it's been made by a Thing factory
	Thing(bool state)
	: valid_state(state)
	{
		if (++failures < 3)
			throw busy{};
	}

	public:
	bool valid_state;

	// User-constructed Things are always invalid :(
	Thing()
	: valid_state(false)
	{}

	Thing(const Thing&) = default;
	Thing& operator=(const Thing&) = default;
};

/// The Maker of Things: Factory 2, electric boogaloo
Thing make_thing()
{ return Thing(true);}

}// namespace stupid_lib

int main()
{
	assert(exception_occurs(throw int{} ));
	assert(not exception_occurs(f_never_throws()));
	assert(exception_occurs(f_always_throws_runtime_error()));

	assert(specific_exception_occurs(int, throw int{} ));
	assert(not specific_exception_occurs(std::exception, f_never_throws() ));
	assert(specific_exception_occurs(std::exception, f_always_throws_runtime_error() ));
	assert(specific_exception_occurs(std::runtime_error, f_always_throws_runtime_error() ));

	bool got_runtime_error{ false };
	try
	{
		assert(not specific_exception_occurs(std::logic_error, f_always_throws_runtime_error()));
	}
	catch(std::runtime_error&)
	{
		// we get here because the wrapper would only catch std::logic_error
		// std::runtime_error is propagated out of the wrapper
		got_runtime_error = true;
	}
	assert(got_runtime_error);

	// An example of dealing with a stupid_lib::Thing factory
	stupid_lib::Thing thing;

	while (specific_exception_occurs(
		stupid_lib::busy,
		thing = stupid_lib::make_thing()))
	{
		// pretend to wait a bit
	}
	assert(thing.valid_state);

	// we get here :)
}

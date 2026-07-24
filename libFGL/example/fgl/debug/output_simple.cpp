/**
This file is an example for <fgl/debug/output.hpp>

--- Example output
-------------------------------------------------------------------------------
[GENERIC] Hello, World! :)
[my with_overload channel] with_overload: my_int = 42, my_double = 3.14159
[GENERIC] Uh-oh, we got a logic error :(


        ATTENTION!
        Generated from: static std::string fgl::debug::output_config<T>
		::default_formatter(const T&)

        The generic `output_config` formatter is only suitable
        for types which satisfy `fgl::traits::printable` by
        having an ostream `operator<<` overload. If you are
        seeing this, you should either define such an overload,
        or define a custom formatter for `T` and assign it to the
        static `fgl::debug::output_config<T>::format` member  variable.


[It'll work this time!] without_overload: my_int = 42, my_double = 3.14159
*/

#include <stdexcept> // logic_error
#include <string>

#include <fgl/debug/output.hpp>

/// This example shows how to customize the generic output handler

// the user-defined type with an ostream operator<< overload
struct with_overload
{
	int my_int;
	double my_double;
};

// an ostream operator<< overload for the user-defined type
std::ostream& operator<<(std::ostream& os, const with_overload& my_struct)
{
	os
		<< "with_overload:"
		<< " my_int = " << my_struct.my_int
		<< ", my_double = " << my_struct.my_double;
	return os;
}

// a user defined type without an ostream operator<< overload
struct without_overload
{
	int my_int;
	double my_double;
};

// a custom formatter
std::string my_formatter(const without_overload& my_struct)
{
	std::stringstream ss;
	ss
		<< "without_overload:"
		<< " my_int = " << my_struct.my_int
		<< ", my_double = " << my_struct.my_double;
	return ss.str();
}

////////////////
/// Entry Point
////////

int main()
{
	// Generic output handler, default formatter, `const char[17]`
	fgl::debug::output("Hello, World! :)");

	// Customize channel name for `with_overload`
	fgl::debug::output_config<with_overload>::name("my with_overload channel");

	// Generic output handler, default formatter, `with_overload`
	with_overload with{ 42, 3.14159265358979323846 };
	fgl::debug::output(with);

	// Generic output handler, default formatter, `without_overload`
	without_overload without{ 42, 3.14159265358979323846 };
	try
	{
		fgl::debug::output(without); // uh-oh...
	}
	catch (const std::logic_error& e)
	{
		std::cout
			<< "Uh-oh, we got a logic error :(\n\t"
			<< e.what()
			<< std::endl;
	}

	// Customize the channel name for `without_overload`
	fgl::debug::output_config<without_overload>::name("It'll work this time!");

	// let's try setting our custom formatter instead...
	fgl::debug::output_config<without_overload>::format = my_formatter;

	fgl::debug::output(without); // :)

	// make sure we flush the stream since the program terminates after this
	fgl::debug::output::stream.flush();
}

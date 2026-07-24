/**
This file is an example for <fgl/types/not_null.hpp>

--- Example output
-------------------------------------------------------------------------------


Let's give the library a string!
uh oh! invalid argument: `fgl::not_null` has equality with `nullptr`
Let's try that again...
[LIB] has taken ownership of "a string from the user"


Let's try to get an int* from the library!
uh oh! int ptr was null: `fgl::not_null` has equality with `nullptr`
Let's try again... Yay! int is 1337


Now let's show what not to do: invalidation!
We messed up :( T_check_on_access saved us!
runtime error: `fgl::not_null` has equality with `nullptr`

Let's do that again but turn off T_check_on_access.
We messed up big time! T_check_on_access can't save us!
not_null is null! Dereferencing would be underfined behavior!

toggle_null has a side effect.
Access toggle_null once: OK!
Access toggle_null again: T_check_on_access saved us again.


Zero overhead assertions!
Be aware: assertions are only done if NDEBUG isn't defined.
Since NDEBUG isn't defined, let's intentionally crash!
Assertion failed: (value != nullptr), file ../../include/fgl/types/not_null.hpp, line 102
*/

#include <new>
#include <iostream>
#include <string>
#include <memory> // unique_ptr
#include <fgl/types/not_null.hpp>

/// A pretend library interface
namespace lib {

/// First call simulates a failure which returns nullptr
int* get_int_ptr() noexcept
{
	static int x{ -1 };
	if (x == -1)
	{
		x = 0;
		return nullptr;
	}
	else return &x;
}

/// An example interface that takes a unique_ptr which must not be null
void take_ownership(fgl::not_null<std::unique_ptr<std::string>> p)
{ std::cout << "[LIB] has taken ownership of \"" << *p << "\"\n"; }

/// Contrived example to demo access checks
// This structure has a side effect; every time we call operator*(), it
// will toggle whether or not it has equality with nullptr
struct toggle_null
{
	bool am_i_null{ false };

	auto& operator*() noexcept
	{
		am_i_null = !am_i_null;
		return am_i_null;
	}

	// test equality with nullptr
	bool operator==(std::nullptr_t) const noexcept
	{ return am_i_null; }
};

} // namespace lib




/// Makes sure the library doesn't give us something thats null!
void safely_get_an_int_ptr_from_lib()
{
	std::cout << "\n\nLet's try to get an int* from the library!\n";
	try
	{
		fgl::not_null p{ lib::get_int_ptr() };
		*p = 42;
	}
	catch (const std::invalid_argument& e)
	{ std::cout << "uh oh! int ptr was null: " << e.what() << std::endl; }
	std::cout << "Let's try again... ";
	try
	{
		fgl::not_null p{ lib::get_int_ptr() };
		*p = 1337; // note: by default, no access check. redundant for raw ptr
		std::cout << "Yay! int is " << *p;
	}
	catch (const std::invalid_argument& e)
	{ std::cout << "shouldnt see this"; }
	std::cout << std::endl;
}


/// The library's interface makes sure we can't give it something thats null
void give_lib_a_string()
{
	std::cout << "\n\nLet's give the library a string!\n";
	try
	{
		std::unique_ptr<std::string> p{};
		lib::take_ownership(std::move(p)); // implicit construction of not_null
	}
	catch (const std::invalid_argument& e)
	{ std::cout << "uh oh! invalid argument: " << e.what() << std::endl; }

	std::cout << "Let's try that again..." << std::endl;

	try
	{
		auto p{ std::make_unique<std::string>("a string from the user") };
		lib::take_ownership(fgl::not_null{ std::move(p) }); // explicit
	}
	catch (const std::invalid_argument& e)
	{
		std::cout
			<< "If you see this, then something went terribly wrong"
			<< std::endl;
	}
}


/// This demonstrates the dangers of invalidation
void invalidation()
{
	std::cout << "\n\nNow let's show what not to do: invalidation!\n";
	// lets make a unique_ptr who isn't null
	fgl::not_null safe{ std::unique_ptr<int>{ new(std::nothrow) int{ 42 } } };
	delete safe.unsafe_reference().release(); // D: oh no
	// safe is now invalid; its null!
	try { *safe = 42; }
	catch (const std::runtime_error& e)
	{
		std::cout
			<< "We messed up :( T_check_on_access saved us!"
			"\nruntime error: " << e.what() << std::endl;
	}

	std::cout << "\nLet's do that again but turn off T_check_on_access.\n";
	fgl::not_null<std::unique_ptr<int>, false, false> dangerous{
		std::unique_ptr<int>{ new(std::nothrow) int{ 69 } }
	};
	delete dangerous.unsafe_reference().release(); // D: oh no
	if (dangerous.reference() == nullptr)
	{
		std::cout
			<< "We messed up big time! T_check_on_access can't save us!"
			"\nnot_null is null! Dereferencing would be underfined behavior!"
			<< std::endl;
	}

	std::cout << "\ntoggle_null has a side effect.\n";

	fgl::not_null<lib::toggle_null> p2;
	std::cout << "Access toggle_null once: ";
	(void)*p2;
	std::cout << "OK!\nAccess toggle_null again: ";
	try { (void)*p2; }
	catch (const std::runtime_error&)
	{ std::cout << "T_check_on_access saved us again."; }
	std::cout << std::endl;
}


void zero_overhead_assertions()
{
	std::cout << "\n\nZero overhead assertions!\n";
	// this is an alias for fgl::not_null where `T_use_assertions = true`
	static_assert(
		std::same_as<
			fgl::assert_not_null<int*>,
			fgl::not_null<int*, true> // T_use_assertions is true
		>
	);
	fgl::assert_not_null p1{
		std::unique_ptr<int>{ new(std::nothrow) int{ 1337 } }
	};
	std::cout
		<< "Be aware: assertions are only done if NDEBUG isn't defined."
		"\nSince NDEBUG isn't defined, let's intentionally crash!"
		<< std::endl;
	fgl::assert_not_null<int*> p2{ nullptr };
}




int main()
{
	// Example of using an interface that takes not_null
	give_lib_a_string();

	// Example of using not_null to check the return of a function
	safely_get_an_int_ptr_from_lib();

	// What not to do: invalidation & dangers of not_null
	invalidation();

	// zero overhead checks
	zero_overhead_assertions(); // program crashes from assertion failure here
}

#include <ranges> // range_value_t
#include <type_traits> // invoke_result_t
#include <algorithm> // copy
#include <array>


/*
	Crossing the compile-time <-> runtime boundary can be an
	unintuitive challenge for memory management.

	Most people want to slap `constexpr` in front of a vector or
	string and be done with it; but it isn't that simple.

	std::vector's storage can't persist to runtime.
	The vector merely owns dynamic / system heap memory.
	All dynamic memory in compile-time contexts must be freed.
	To persist the data, the dynamic regions must be flattened
	to fixed-length static array
	(mutable .data constinit or const .rodata constexpr)

	In an ideal world, you could have a function...
	`consteval auto crystallize(constexpr std::vector<T> vec)`
	however, there are two problems:
	1. function parameters can't be constexpr
	2. we need a constexpr size for the array template

	Point 2 is of particular importance, because we often
	don't know what the size will be until we're finished
	our compile-time computations.

	This example strategy uses a lambda-type range factory
	so that we can perform the compile-time operations twice:
	once to determine the size of array we need, and once more
	to compute the values we will populate it with.

	At the time of writing, I know of no way to get a constexpr
	size without duplication of effort. This also means that the
	computations must be deterministic; identical invocations of
	the factory must produce the same length. Having the factory
	be a lambda passed by templates helps with this, a little.
	It's also just generally more convenient.

	This approach should work with any sized range, however,
	there can be complications due to class composition and
	object lifetimes, move/copy ctors, etc.
	- This is a naive example.
	- Omitting template constraints for readability...
*/

// helps us deduce the underlying type produced by the range factory
template <typename T>
using typer = std::ranges::range_value_t<std::invoke_result_t<T>>;

// finds the size by doing work and discarding the result :(
template <typename T>
constexpr std::size_t sizer{ std::ranges::size(T{}()) };

// does the work, and copies the dynamic range to a fixed array
template <typename T>
consteval auto crystallizer()
{
	std::array<typer<T>, sizer<T>> out{};
	auto&& temp{ T{}() };
	std::copy(temp.begin(), temp.end(), out.begin());
	return out;
}

// lambda-wrapping call helper; not needed but otherwise a bit ugly.
#define CRYSTALLIZE(...) crystallizer<decltype([]{ __VA_ARGS__; })>()

// Produces a vector from an array.
// I'll leave the std::string equivalent as an exercise :P
template <typename T_in, std::size_t T_size>
std::vector<T_in> hydrate(const std::array<T_in, T_size>& in)
{
	std::vector<T_in> out;
	out.resize(T_size);
	std::copy(in.begin(), in.end(), out.begin());
	return out;
}



/// example usage



#include <iostream>
#include <vector>

constexpr std::vector<int> func(int count)
{
	std::vector<int> v;
	for (int i{ 0 }; i < count; ++i)
		v.push_back(i);
	return v;
}

int main()
{
	static constexpr auto array1{
		CRYSTALLIZE(
			std::string s1 = "Hello, ";
			std::string s2 = "World!\n";
			return s1 + s2;
		)
	};
	for (char c : array1)
		std::cout << c;

	static constexpr auto array2{ CRYSTALLIZE( return func(3) ) };
	std::cout << array2.size() << '\n';

	// since crystallizer is consteval, we can do it in-line
	// and still benefit from the static storage duration
	auto vec{ hydrate( CRYSTALLIZE( return func(5) ) ) };
	for (auto& e : vec) // runtime vector initialized from constexpr work
		std::cout << e << ", ";
}

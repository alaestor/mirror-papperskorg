/**
This file is an example for <fgl/utility/circular_iterator.hpp>

--- Example output
-------------------------------------------------------------------------------
-9      -> 0,
-8      -> 1,
-7      -> 2,
-6      -> 0,
-5      -> 1,
-4      -> 2,
-3      -> 0,
-2      -> 1,
-1      -> 2,
0       -> 0,
1       -> 1,
2       -> 2,
3       -> 0,
4       -> 1,
5       -> 2,
6       -> 0,
7       -> 1,
8       -> 2,
9       -> 0,

95, 96, 97, 98, 99,
*/

#include <cassert>
#include <array>
#include <iostream>
#include <fgl/utility/circular_iterator.hpp>

void circular_index_wrapping()
{
	constexpr int range_size{ 3 }; // our pretend range
	constexpr int bound{ range_size * 3 }; // indexes beyond range_size wrap
	for (int i{ -bound }; i != bound+1; ++i)
	{
		const auto index{ fgl::wrap_index(i, range_size) };
		assert(index >= 0 && index < range_size); // wrapped is within range
		std::cout << i << "\t-> " << index << ",\n";
	}
	std::cout << std::endl;
}

void circular_iterator()
{
	std::array<int, 5> arr{};
	fgl::circular_iterator iter{ arr.begin(), arr.end(), arr.begin() };

	iter += arr.size(); // past end, wrap around.
	assert(iter.cursor() == arr.begin());

	iter -= 1; // before beginning, wrap around.
	assert(iter.cursor() == arr.end() - 1);

	int size{ static_cast<int>(arr.size()) };

	assert(&iter[size * 50] == &arr[0]); // wraps over many times
	assert(&iter[size * 50 * -1] == &arr[0]); // wraps under many times
}


void circular_range()
{
	std::array<int, 5> arr{};
	auto alias{ fgl::make_circular_range_alias(arr) };

	// the range's `begin` iterator is equivalent to this circular iterator
	assert((
		alias.begin()
		== fgl::circular_iterator{ arr.begin(), arr.end(), arr.begin() }
	));

	// the range's `end` is unreachable
	static_assert(
		std::same_as<decltype(alias.end()), std::unreachable_sentinel_t>
	);

	// this would be an infinite loop without the `break`
	for (int counter{}; auto& elem : alias)
	{
		if (counter == 100)
			break;

		elem = counter++;
	}

	for (const int elem : arr)
		std::cout << elem << ", ";
	std::cout << std::endl;
}

int main()
{
	circular_index_wrapping();
	circular_iterator();
	circular_range();
}

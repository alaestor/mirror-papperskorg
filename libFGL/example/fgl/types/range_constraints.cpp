/**
This file is an example for <fgl/types/range_cnstraints.hpp>

--- Example output
-------------------------------------------------------------------------------
a: 1 2 3 4 5 6 7
b: 7 6 5 4 3 2 1
swap...
a: 7 6 5 4 3 2 1
b: 1 2 3 4 5 6 7
*/

#include <iostream>
#include <utility> // swap
#include <ranges> // begin, end
#include <vector>
#include <array>

#include <fgl/types/range_constraints.hpp>

template <std::ranges::range T_range>
void simple_range_swap(
	T_range& r1,
	fgl::range_swappable_with_range<T_range> auto& r2)
{
	auto r1_iter{ std::ranges::begin(r1) };
	const auto r1_end{ std::ranges::end(r1) };
	auto r2_iter{ std::ranges::begin(r2) };
	const auto r2_end{ std::ranges::end(r2) };

	while (r1_iter != r1_end && r2_iter != r2_end)
		std::iter_swap(r1_iter++, r2_iter++);
}

void print_contiguous_int_range(
	const auto& prefix,
	const fgl::contiguous_range_same_as<int> auto& range)
{
	std::cout << prefix;
	for (const int v : range)
		std::cout << v << ' ';
	std::cout << '\n';
}

int main()
{
	std::vector a{ 1,2,3,4,5,6,7 };
	std::array b{ 7,6,5,4,3,2,1 };
	print_contiguous_int_range("a: ", a);
	print_contiguous_int_range("b: ", b);
	std::cout << "swap...\n";
	simple_range_swap(a, b);
	print_contiguous_int_range("a: ", a);
	print_contiguous_int_range("b: ", b);
	std::cout << std::endl; // flush buffer
}

/**
This file is an example for <fgl/utility/linear_matrix_alias/dynamic.hpp>

--- Example output
-------------------------------------------------------------------------------
row major 3x3x3:
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,

column major 4x4x4:
0, 16, 32, 48, 4, 20, 36, 52, 8, 24, 40, 56, 12, 28, 44, 60, 1, 17, 33, 49, 5, 21, 37, 53, 9, 25, 41, 57, 13, 29, 45, 61, 2, 18, 34, 50, 6, 22, 38, 54, 10, 26, 42, 58, 14, 30, 46, 62, 3, 19, 35, 51, 7, 23, 39, 55, 11, 27, 43, 59, 15, 31, 47, 63,
*/

#include <cstddef> // size_t
#include <iostream> // cout, endl
#include <vector>

#include <fgl/utility/linear_matrix_alias/dynamic.hpp>

int main()
{
	std::size_t x{ 3 }, y{ 3 }, z{ 3 };
	std::vector<short> buffer;
	buffer.resize(x*y*z);
	for (short i{}; auto& v : buffer)
		v = i++; // fill buffer

	using enum fgl::major;
	auto alias{ fgl::make_matrix_alias(buffer, row, x,y,z) };

	std::cout << "row major " << x << 'x' << y << 'x' << z << ":\n";
	for (std::size_t i{}; i < x; ++i)
	for (std::size_t j{}; j < y; ++j)
	for (std::size_t l{}; l < z; ++l)
	{
		std::cout << alias[{i, j, l}] << ", ";
	}

	++x, ++y, ++z;
	buffer.resize(x*y*z);
	for (short i{}; auto& v : buffer)
		v = i++; // fill buffer

	alias.update(buffer, {x,y,z}, column);

	std::cout << "\n\ncolumn major " << x << 'x' << y << 'x' << z << ":\n";
	for (std::size_t i{}; i < x; ++i)
	for (std::size_t j{}; j < y; ++j)
	for (std::size_t l{}; l < z; ++l)
	{
		std::cout << alias[{i, j, l}] << ", ";
	}
	std::cout << std::endl;
}

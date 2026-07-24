/**
This file is an example for <fgl/utility/linear_matrix_alias/static.hpp>

--- Example output
-------------------------------------------------------------------------------
row major: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
column major: 0, 9, 18, 3, 12, 21, 6, 15, 24, 1, 10, 19, 4, 13, 22, 7, 16, 25, 2, 11, 20, 5, 14, 23, 8, 17, 26,
*/

#include <cstddef> // size_t
#include <iostream> // cout, endl
#include <array>

#include <fgl/utility/linear_matrix_alias/static.hpp>

int main()
{
	constexpr std::size_t x{3}, y{3}, z{3};
	std::array<short, x*y*z> buffer;
	for (short i{}; auto& v : buffer)
		v = i++; // fill buffers with linear indexes

	using enum fgl::major;
	auto alias_row{ fgl::make_matrix_alias<row, x,y,z>(buffer) };
	auto alias_column{ fgl::make_matrix_alias<column, x,y,z>(buffer) };

	// The above is equivalent to:
	// using iter_t = decltype(buffer.begin());
	// fgl::static_linear_matrix_alias<iter_t, row, x,y,z> alias_row(buffer);
	// fgl::static_linear_matrix_alias<iter_t, column, x,y,z> alias_column(buffer);

	std::cout << "row major: ";
	for (std::size_t i{}; i < x; ++i)
	for (std::size_t j{}; j < y; ++j)
	for (std::size_t l{}; l < z; ++l)
	{
		std::cout << alias_row[{i, j, l}] << ", ";
	}

	std::cout << "\ncolumn major: ";
	for (std::size_t i{}; i < x; ++i)
	for (std::size_t j{}; j < y; ++j)
	for (std::size_t l{}; l < z; ++l)
	{
		std::cout << alias_column[{i, j, l}] << ", ";
	}
	std::cout << std::endl;
}

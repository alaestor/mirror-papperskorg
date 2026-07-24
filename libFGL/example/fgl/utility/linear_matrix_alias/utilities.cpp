/**
This file is an example for <fgl/utility/linear_matrix_alias/utilities.hpp>

--- Example output
-------------------------------------------------------------------------------
row major:
  0,   1,   2,
  3,   4,   5,
  6,   7,   8,

column major:
  0,   3,   6,
  1,   4,   7,
  2,   5,   8,

row major:
  0,   1,   2,   3,
  4,   5,   6,   7,
  8,   9,  10,  11,
 12,  13,  14,  15,

column major:
  0,   4,   8,  12,
  1,   5,   9,  13,
  2,   6,  10,  14,
  3,   7,  11,  15,
*/

#include <cstddef> // size_t
#include <iostream> // cout, endl
#include <iomanip> // setw
#include <array>
#include <vector>

#include <fgl/utility/linear_matrix_alias/utilities.hpp>

void fill_buffer(auto& buffer)
{
	for (std::size_t i{}; auto& v : buffer)
		v = i++;
}

void print_2d_matrix(
	const auto& buffer,
	const auto& bounds,
	const auto& offsets)
{
	using fgl::linear_matrix_utilities::matrix_to_linear;
	for (std::size_t i{}; i < bounds[0]; ++i)
	{
		for (std::size_t j{}; j < bounds[1]; ++j)
		{
			std::cout
				<< std::setw(3)
				<< buffer[matrix_to_linear(std::array{i,j}, offsets)]
				<< ", ";
		}
		std::cout << '\n';
	}
	std::cout << std::endl;
}

void print_2d_matrix_both_majors(const auto& buffer, const auto& bounds)
{
	std::vector<std::size_t> offsets(bounds.size());

	using fgl::linear_matrix_utilities::calculate_offsets;

	std::cout << "row major:\n";
	calculate_offsets<0, fgl::major::row>(bounds, offsets);
	print_2d_matrix(buffer, bounds, offsets);

	std::cout << "column major:\n";
	calculate_offsets<0, fgl::major::column>(bounds, offsets);
	print_2d_matrix(buffer, bounds, offsets);
}

int main()
{
	std::vector<std::size_t> bounds{3,3}; // 2D square, 3x3
	std::vector<std::size_t> buffer(3*3);

	fill_buffer(buffer);
	print_2d_matrix_both_majors(buffer, bounds);

	// resize square 3x3 -> 4x4
	bounds[0] = 4; bounds[1] = 4;
	buffer.resize(4*4);

	fill_buffer(buffer);
	print_2d_matrix_both_majors(buffer, bounds);
}

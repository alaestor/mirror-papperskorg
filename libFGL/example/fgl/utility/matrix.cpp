/**
This file is an example for <fgl/utility/matrix.hpp>

--- Example output
-------------------------------------------------------------------------------
Matrix 5x5:
  0,   1,   2,   3,   4,
  5,   6,   7,   8,   9,
 10,  11,  12,  13,  14,
 15,  16,  17,  18,  19,
 20,  21,  22,  23,  24,
*/

#include <cstddef> // size_t
#include <iostream> // cout, endl
#include <iomanip> // setw

#include <fgl/utility/matrix.hpp>

int main()
{
	constexpr std::size_t x{ 5 }, y{ 5 };
	fgl::matrix<short, x, y> matrix;

	for (short i{}; auto& v : matrix) // using std::array's begin/end
		v = i++; // fill matrix linearly, since its row major

	std::cout << "Matrix " << x << 'x' << y << ":\n";
	for (std::size_t i{}; i < x; ++i)
	{
		for (std::size_t j{}; j < y; ++j)
		{
			std::cout << std::setw(3) << matrix[{i, j}] << ", ";
		}
		std::cout << std::endl;
	}
}

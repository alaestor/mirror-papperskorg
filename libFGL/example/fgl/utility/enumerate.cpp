/**
This file is an example for <fgl/utility/enumerate.hpp>

--- Example output
-------------------------------------------------------------------------------
arr[0] == 1
vec[0] == 3.14

arr[1] == 2
vec[1] == 3.14

arr[2] == 3
vec[2] == 3.14

arr[3] == 4
vec[3] == 3.14
*/

#include <array>
#include <vector>
#include <iostream>

#include <fgl/utility/enumerate.hpp>

int main()
{
	const std::array arr{ 1,2,3,4 };
	std::vector<float> vec(8, 3.14f);

	for (const auto& [i, v1, v2] : fgl::cenumerate(arr, vec))
	{
		std::cout << "arr" << '[' << i << "] == " << v1 << '\n';
		std::cout << "vec" << '[' << i << "] == " << v2 << '\n';
		std::cout << std::endl;
	}
}

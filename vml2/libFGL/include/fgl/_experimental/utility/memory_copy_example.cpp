/**
This file is an example for <fgl/utility/memory_copy.hpp>

--- Example output
-------------------------------------------------------------------------------
copy_arr == source? true
copy_i2: 39210
copy_back == source? true
compile time: 39210
*/

#include <iostream>
#include <iomanip>
#include <cstddef> // byte
#include <cstring> // memcpy for example
#include <ranges> // subrange
#include <array>

#include <fgl/utility/memory_copy.hpp>

constexpr std::array<int, 5> source{ -1, -5059, 39210, 0, 50 };

int main()
{
	std::array<int, 5> copy_arr{};
	// this gets length to copy from the sizeof copy_arr
	fgl::unsafe::memory_copy(&copy_arr, source.data());
	std::cout
		<< "copy_arr == source? "
		<< std::boolalpha << (copy_arr == source)
		<< '\n';

	int copy_i2{};
	fgl::memory_copy(&copy_i2, &source[2]);
	std::cout << "copy_i2: " << copy_i2 << '\n';

	std::ranges::subrange binary_range{
		reinterpret_cast<const std::byte*>(&*source.cbegin()),
		reinterpret_cast<const std::byte*>(&*source.cend())
	};

	std::array<int, 5> copy_back{};
	fgl::memory_copy(&copy_back, binary_range);
	std::cout
		<< "copy_back == source? "
		<< std::boolalpha << (copy_back == source)
		<< '\n';

	constexpr unsigned int compiletime{
		[]() consteval -> unsigned int
		{
			unsigned int temp{};
			fgl::memory_copy(&temp, &source[2]); // uses the bitcast overload
			return temp;
		}()
	};
	std::cout << "compile time: " << compiletime;
	std::cout << std::endl;
}






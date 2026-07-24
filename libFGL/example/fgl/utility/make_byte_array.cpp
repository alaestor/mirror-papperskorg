/**
This file is an example for <fgl/utility/make_byte_array.hpp>

--- Example output
-------------------------------------------------------------------------------
01 A0 AF FF 20 68 65 6C 6C 6F
01 A0 AF FF 20 67 6F 6F 64 62 79 65
*/

#include <iostream>
#include <iomanip>
#include <cstddef> // byte, to_integer

#include <fgl/utility/make_byte_array.hpp>

int main()
{
	std::cout // hex output
		<< std::hex
		<< std::internal
		<< std::uppercase
		<< std::setfill('0');

	constexpr auto barray1 = fgl::make_byte_array("\x01\xA0\xAF\xFF hello");
	for (const std::byte b : barray1)
		std::cout << std::setw(2) << std::to_integer<unsigned int>(b) << ' ';

	std::cout << std::endl;

	using namespace fgl::literals;
	constexpr auto barray2 = "\x01\xA0\xAF\xFF goodbye"_barray;
	for (const std::byte b : barray2)
		std::cout << std::setw(2) << std::to_integer<unsigned int>(b) << ' ';

	std::cout << std::endl;
}

/*
This file is an example for <fgl/io/binary_files.hpp>

--- Example output
-------------------------------------------------------------------------------
hi
*/

#include <cstddef> // byte
#include <array>
#include <iostream>
#include <filesystem> // path

#include <fgl/io/binary_files.hpp>

int main()
{
	constexpr std::array binary_data{ std::byte{'h'}, std::byte{'i'} };
	const std::filesystem::path file_path{ "file.bin" };

	fgl::write_binary_file(file_path, binary_data);

	const auto contents_in{ fgl::read_binary_file<char>(file_path) };

	for (const char c : contents_in)
		std::cout << c;

	std::cout << std::endl;
}

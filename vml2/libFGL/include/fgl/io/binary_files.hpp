#pragma once
#ifndef FGL_UTILITY_BINARY_FILES_HPP_INCLUDED
#define FGL_UTILITY_BINARY_FILES_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cassert>
#include <ranges> // contiguous_range
#include <string> // string, to_string
#include <stdexcept> // runtime_error
#include <concepts> // integral
#include <vector>
#include <filesystem>
#include <fstream>
#include <optional>
#include <limits>

#include "../types/traits.hpp" // byte_type
#include "../types/range_constraints.hpp" // contiguous_range_byte_type

namespace fgl {

/**
@file

@example example/fgl/io/binary_files.cpp
	An example for @ref group-io-binary_files

@defgroup group-io-binary_files Binary File I/O

@brief Easy input and output for binary files

@details
	@see the example program @ref example/fgl/io/binary_files.cpp

@{
*/

///@cond FGL_INTERNAL_DOCS
namespace internal {

///@internal @brief casting wrapper for <tt>std::filesystem::file_size</tt>
inline std::size_t get_file_size(const std::filesystem::path& path)
{
	// cast isn't useless; <tt>std::uintmax_t</tt> may differ from <tt>std::size_t</tt>
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wuseless-cast"
	return static_cast<std::size_t>(std::filesystem::file_size(path));
	#pragma GCC diagnostic pop
}

} // namespace internal
///@endcond

/**
@brief Reads a binary file into a contiguous range of bytes
@param file_path The path to the file to read
@param[out] output_range A contiuogus range of bytes to write the file
	contents to. It's size must be greater than or equal to
	<tt>bytes_to_read</tt>, or the size of the file if <tt>bytes_to_read</tt>
	is <tt>0</tt>. For concept details, refer to
	@ref group-types-range_constraints.
@param bytes_to_read The number of bytes to read from the file. If this is
	<tt>0</tt>, the size of the file will be used. This must be less than or
	equal to the maximum representable value of <tt>std::streamsize</tt>.
@note it's asserted that <tt>bytes_to_read</tt>, or the size of the file if
	<tt>bytes_to_read</tt> is <tt>0</tt>, is less than or equal to the size of
	the maximum representable value of the type of <tt>std::streamsize</tt>.
@returns The number of bytes read from the file. This will always be
	<tt>bytes_to_read</tt> unless <tt>bytes_to_read</tt> is <tt>0</tt>, in
	which case it will be the size of the file.
@throws std::filesystem::filesystem_error If <tt>bytes_to_read</tt> was
	<tt>0</tt> and the file size couldn't be obtained.
@throws std::runtime_error if the buffer is less than <tt>bytes_to_read</tt>,
	or the file couldn't be opened via <tt>std::ifstream</tt>
@throws [various] Standard <tt>std::ifstream</tt> exceptions which are enabled
	for <tt>badbit</tt>, <tt>failbit</tt>, and <tt>eofbit</tt>.
*/
std::size_t read_binary_file(
	const std::filesystem::path& file_path,
	fgl::contiguous_range_byte_type auto& output,
	const std::size_t bytes_to_read = 0)
	// wish i could write " = get_file_size(file_path)" but no...
{
	const std::size_t buffer_size{ std::ranges::size(output) };
	const std::size_t read_size{
		(bytes_to_read > 0)
		? bytes_to_read
		: internal::get_file_size(file_path)
	};

	assert(
		read_size
		< static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())
	);

	if (buffer_size < read_size)
	{
		std::string estr{ "read_binary_file() failed to read " };
		estr += file_path.string();
		estr += " - the buffer is too small to hold the file contents";
		throw std::runtime_error(estr);
	}

	if (std::ifstream ifs(file_path, std::ios::binary);
		ifs)
	{
		ifs.exceptions(ifs.badbit | ifs.failbit | ifs.eofbit);
		ifs.read(
			reinterpret_cast<char*>(std::ranges::data(output)),
			static_cast<std::streamsize>(read_size)
		);
		return read_size;
	}
	else
	{
		std::string estr{ "read_binary_file() failed to open " };
		estr += file_path.string();
		throw std::runtime_error(estr);
	}
}

/**
@brief Constructs and returns a vector with the contents of a file.
@details This function is a convenience wrapper around
	<tt>@ref read_binary_file()</tt>.
@tparam T The underlying type of the vector which will hold the file contents.
	Must satisfy <tt>@ref fgl::traits::byte_type</tt>
@param file_path The path to the file to read
@returns A <tt>std::vector</tt> of <tt>T</tt> containing the contents of the
	file.
@throws [various] <tt>std::vector</tt> exceptions
@throws [various] <tt>read_binary_file()</tt> exceptions
*/
template <fgl::traits::byte_type T = std::byte> [[nodiscard]]
inline std::vector<T> read_binary_file(const std::filesystem::path& file_path)
{
	const std::size_t file_size{ internal::get_file_size(file_path) };
	std::vector<T> byte_buffer(file_size, T{});
	read_binary_file(file_path, byte_buffer, file_size);
	return byte_buffer;
}

/**
@brief Writes a number of bytes from a contiguous range of bytes to a binary
	file.
@param file_path The path to the file to write
@param input A contiguous range of bytes to write to the file For concept
	details, refer to @ref group-types-range_constraints.
@param bytes_to_write The number of bytes to write to the file. Must be less
	than or equal to the size of the input range, and less than the maximum
	representable value of <tt>std::streamsize</tt>.
@param mode The mode to open the file with. Defaults to
	<tt>std::ios::trunc</tt>.
@note it's asserted that <tt>bytes_to_write</tt> is less than the maximum
	representable value of the type of <tt>std::streamsize</tt>.
@throws std::runtime_error if the file couldn't be opened via
	<tt>std::ofstream</tt>
@throws std::invalid_argument if <tt>bytes_to_write</tt> is greater than the
	size of the input range
@throws [various] Standard <tt>std::ofstream</tt> exceptions which are enabled
	for <tt>badbit</tt> and <tt>failbit</tt>.
*/
void write_binary_file(
	const std::filesystem::path& file_path,
	const fgl::contiguous_range_byte_type auto& input,
	const std::size_t bytes_to_write,
	const std::ios::openmode mode = std::ios::trunc)
{
	assert(
		bytes_to_write
		< static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())
	);

	if (bytes_to_write > std::ranges::size(input))
	{
		std::string estr{ "write_binary_file() failed to write " };
		estr += file_path.string();
		estr += " - the number of bytes to write must be less than or equal to"
			" the size of the input range";
		throw std::invalid_argument(estr);
	}

	if (std::ofstream ofs(file_path, std::ios::binary | mode);
		ofs)
	{
		ofs.exceptions(std::ios::badbit | std::ios::failbit);
		ofs.write(
			reinterpret_cast<const char*>(std::ranges::cdata(input)),
			static_cast<std::streamsize>(bytes_to_write)
		);
	}
	else
	{
		std::string estr{ "write_binary_file() couldn't open " };
		estr += file_path.string();
		throw std::runtime_error(estr);
	}
}

/**
@brief Writes a contiguous range of bytes to a binary file.
@details This function is a convenience wrapper around
	<tt>@ref write_binary_file()</tt> and is equivalent to calling
	it with the size of <tt>input</tt> as <tt>bytes_to_write</tt>.
@param file_path The path to the file to write
@param input A contiguous range of bytes to write to the file. For concept
	details, refer to @ref group-types-range_constraints.
@param mode The mode to open the file with. Defaults to
	<tt>std::ios::trunc</tt>.
@throws [various] exceptions from <tt>write_binary_file()</tt>
*/
inline void write_binary_file(
	const std::filesystem::path& file_path,
	const fgl::contiguous_range_byte_type auto& input,
	const std::ios::openmode mode = std::ios::trunc)
{
	write_binary_file(file_path, input, std::ranges::size(input), mode);
}

///@} group-io-binary_files

}// namespace fgl

#endif // FGL_UTILITY_BINARY_FILES_HPP_INCLUDED

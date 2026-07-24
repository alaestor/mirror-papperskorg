#pragma once
#ifndef XFILE_SECTION_H_INCLUDED
#define XFILE_SECTION_H_INCLUDED

#include <string>
#include <string_view>
#include <vector>
#include <cstddef> // byte, size_t
#include <stdexcept> // invalid_argument
#include <fstream> // ifstream

namespace xfile {
namespace internal {

struct Section
{
	const std::string name;
	const std::vector<std::byte> buffer;

	[[nodiscard]]
	std::size_t serialized_size() const noexcept;

	void serialize(std::vector<std::byte>& out_buffer) const;
	
	[[nodiscard]]
	static Section deserialize(std::ifstream& ifst);

	explicit Section(
		const std::string in_name,
		const std::vector<std::byte>& in_data
	): name(in_name), buffer(in_data)
	{
		if (in_name.empty())
			throw std::invalid_argument("name string must not be empty");
	}

	Section() = delete;
	
};

}// namespace internal
}// namespace xfile

#endif // XFILE_SECTION_H_INCLUDED
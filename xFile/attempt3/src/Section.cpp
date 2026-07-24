#include <string>
#include <vector>
#include <cstddef> // byte, size_t
#include <fstream> // ifstream
//#include <stdexcept>
//#include <utility> // move
//#include <memory> // uique_ptr

#include "../hdr/section.hpp"

namespace xfile {
namespace internal {
	
/*
	Serialized sections are read back to front due to the manifest
	being appended to the end of the file.
	
	
	Serialized section format:
		Data, std::byte[]
		Length of data, std::size_t
		Name, null-terminated char[]
		Length of name including null-term, std::size_t
		Total Length of serialized section, std::size_t
*/

std::size_t Section::serialized_size() const noexcept
{
	return
		buffer.size() // Data, std::byte[]
		+sizeof(std::size_t) // Length of data, std::size_t
		+name.length()+1 // Name, null-terminated char[]
		+sizeof(std::size_t) // Length of name including null-term, std::size_t
		+sizeof(std::size_t); // Total Length of serialized section, std::size_t
}

void Section::serialize(std::vector<std::byte>& out_buffer) const
{
	auto append{
		[&out_buffer](const void* ptr, const std::size_t size)
		{ // note std::copy back_inserter is slower, insert is nearly memcpy
			auto bPtr{ reinterpret_cast<const std::byte*>(ptr) };
			out_buffer.insert(out_buffer.end(), bPtr, bPtr+size);
		}
	};

	// Data, std::byte[]
	const std::size_t dataLen{ buffer.size() };
	if (dataLen != 0)
		append(buffer.data(), dataLen);
	
	// Length of data, std::size_t
	append(&dataLen, sizeof(dataLen));
	
	// Name, null-terminated char[]
	const std::size_t nameLen{ name.length()+1 };
	append(name.c_str(), nameLen);
	
	// Length of name including null-term, std::size_t
	append(&nameLen, sizeof(nameLen));
	
	// Total Length of serialized section, std::size_t
	const std::size_t sectionSize{ serialized_size() };
	append(&sectionSize, sizeof(sectionSize));
}

Section Section::deserialize(std::ifstream& ifst)
{
	auto backwards_read_toValue{
		[&ifst]<typename T>([[maybe_unused]] const T& o) -> T
		{
			auto s{ static_cast<long long int>(sizeof(T)) };
			ifst.seekg(-s, std::ios_base::cur);
			T t;
			ifst.read(reinterpret_cast<char*>(&t), s);
			ifst.seekg(-s, std::ios_base::cur);
			return t;
		}
	};
	
	auto backwards_read_toBuffer{
		[&ifst](void* buff_out, const long long int len)
		{
			if (len < 1) throw std::invalid_argument(
				"backwards_read_toBuffer length must be > 0");

			ifst.seekg(-len, std::ios_base::cur);
			ifst.read(reinterpret_cast<char*>(buff_out), len);
			ifst.seekg(-len, std::ios_base::cur);
		}
	};
	
	// Total Length of serialized section, std::size_t
	const std::size_t sectionSize{ backwards_read_toValue(sectionSize) };

	// Length of name including null-term, std::size_t
	const std::size_t nameLen{ backwards_read_toValue(nameLen) };
	
	// Name, null-terminated char[]
	std::vector<char> name_vec;
	name_vec.resize(nameLen);
	backwards_read_toBuffer(
		name_vec.data(), static_cast<long long int>(name_vec.size()));
	const char* name_cstr{ name_vec.data() };
	
	// Length of data, std::size_t
	const std::size_t dataLen{ backwards_read_toValue(dataLen) };

	// Data, std::byte[]
	std::vector<std::byte> tmpBuff;
	if (dataLen != 0)
	{
		tmpBuff.resize(dataLen);
		backwards_read_toBuffer(
			tmpBuff.data(), static_cast<long long int>(tmpBuff.size()));
	}
	
	return Section(name_cstr, tmpBuff);
}

}// namespace internal
}// namespace xfile
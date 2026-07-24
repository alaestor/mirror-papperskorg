#include <string>
#include <vector>
#include <cstddef> // byte, size_t
#include <stdexcept> // invalid_argument
#include <filesystem> // path
#include <fstream> // ifstream, ofstream

#include "../hdr/manifest.hpp"
#include "../hdr/file_utilities.hpp"

namespace xfile {
namespace internal {

void Manifest::delSection(const std::string& name)
{
	if (!sectionExists(name))
		throw std::invalid_argument(name+" section doesn't exist");
	
	m_sections.erase(name);
}
	
void Manifest::addSection(const Section& section)
{
	if (sectionExists(section.name))
		throw std::invalid_argument(section.name+" section already exists");
	
	m_sections.try_emplace(section.name, section);
}

void Manifest::addSection(
		const std::string& name,
		const std::vector<std::byte>& data)
{
	addSection(Section(name, data));
}

std::size_t Manifest::sectionCount() const noexcept
{
	return m_sections.size();
}
	
bool Manifest::sectionExists(const std::string& name) const noexcept
{
	return m_sections.contains(name);
}

const Section& Manifest::getSection(const std::string& name) const
{
	if (!sectionExists(name))
		throw std::invalid_argument(name+" section doesn't exist");
	
	return m_sections.at(name);
}

std::vector<std::byte> Manifest::serialize() const
{
	const std::size_t buffLen{
		[this]()->std::size_t
		{
			std::size_t len{ 0 };
			for (const auto& [_, section] : m_sections)
				len += section.serialized_size();
			return len; // combined size of all serialized sections
		}()
		+ sizeof(std::size_t) // number of sections
		+ sizeof(std::size_t) // length of serialized data
		+ m_magic.length()+1 // magic
	};

	std::vector<std::byte> out_buffer;
	out_buffer.reserve(buffLen);
	
	auto append{
		[&out_buffer](const void* ptr, const std::size_t size)
		{ // note std::copy back_inserter is slower, insert=memcpyish
			auto bPtr{ reinterpret_cast<const std::byte*>(ptr) };
			out_buffer.insert(out_buffer.end(), bPtr, bPtr+size);
		}
	};

	// write all sections
	for (const auto& [_, section] : m_sections)
		section.serialize(out_buffer);
	
	// write section count
	const std::size_t sectionCount{ m_sections.size() };
	append(&sectionCount, sizeof(sectionCount));
	
	// write serialized length
	append(&buffLen, sizeof(buffLen));
	
	// write magic
	append(std::string(m_magic).c_str(), m_magic.length()+1);

	return out_buffer;
}

void Manifest::serializeToFile(const std::filesystem::path filePath) const
{
	delManifestFrom(filePath);
	auto serialized{ serialize() };
	
	if (std::ofstream ofs(filePath.string(), std::ios::binary | std::ios::app);
		ofs)
	{
		ofs.write(
			reinterpret_cast<char*>(serialized.data()),
			static_cast<std::streamsize>(serialized.size())
		);
	}
	else throw std::runtime_error(
		"serializeToFile() couldn't open file: " + filePath.string());
}

std::size_t Manifest::fileManifestSize(std::filesystem::path filePath) // todo const, static, const filepath
{
	if (std::ifstream ifs(filePath.c_str(), std::ios::binary); ifs)
	{
		ifs.seekg(0, std::ios_base::end);
		constexpr const long long magicLen{ m_magic.length()+1 };
		char magicChk[magicLen];
		
		ifs.seekg(-magicLen, std::ios_base::cur);
		ifs.read(reinterpret_cast<char*>(magicChk), magicLen);
		ifs.seekg(-magicLen, std::ios_base::cur);
		
		if (magicChk == m_magic)
		{
			const std::size_t lengthOfManifest{
				[&ifs]() -> std::size_t
				{
					auto s{ static_cast<long long int>(sizeof(std::size_t)) };
					ifs.seekg(-s, std::ios_base::cur);
					std::size_t t;
					ifs.read(reinterpret_cast<char*>(&t), s);
					ifs.seekg(-s, std::ios_base::cur);
					return t;
				}() 
			};
			return lengthOfManifest;
		}
		else return 0; // no manifest found
	}
	else throw std::runtime_error(
		"fileContainsManifest() couldn't open file: " + filePath.string());
}

bool Manifest::fileContainsManifest(std::filesystem::path filePath) // const static constfilepath
{
	return fileManifestSize(filePath) > 0;
}

bool Manifest::delManifestFrom(std::filesystem::path filePath)
{
	const std::size_t manifestSize{ fileManifestSize(filePath) };
	if (manifestSize == 0)
		return false; // no manifest found
	
	const auto totalSize{ file_utilities::sizeOfFile(filePath) };
	std::filesystem::resize_file(filePath, totalSize - manifestSize); // ??
	
	return true;
}

Manifest::Manifest(std::filesystem::path manifest_containing_file)
{
	std::ifstream ifst(manifest_containing_file.c_str(), std::ios::binary);

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
	
	// set cursor to end of file to begin backwards reading serialized data
	ifst.seekg(0, std::ios_base::end);

	// check magic
	char magicChk[m_magic.length()+1];
	backwards_read_toBuffer(&magicChk, sizeof(magicChk));
	
	if (m_magic == magicChk)
	{
		// get manifest metadata
		const std::size_t totalLength{ backwards_read_toValue(totalLength) };
		const std::size_t sectionCount{ backwards_read_toValue(sectionCount) };
		
		// create sections from serialized sections
		for (std::size_t n{ 0 }; n < sectionCount; ++n)
			addSection(Section::deserialize(ifst));
		
	}
	//else throw std::runtime_error("Expected to find magic; not found!");
}

}// namespace internal
}// namespace xfile

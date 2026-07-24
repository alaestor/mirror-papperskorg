#pragma once
#ifndef XFILE_MANIFEST_H_INCLUDED
#define XFILE_MANIFEST_H_INCLUDED

#include <string>
#include <vector>
#include <unordered_map>
#include <cstddef> // byte, size_t
#include <stdexcept> // invalid_argument
#include <filesystem> // std::filesystem::path
#include <fstream>

#include <iostream> // TODO REMOVE DEBUG

#include "../hdr/section.hpp"

/*
WHAT DOES MANIFEST DO?

Manifest has two use cases: input and output.
It's responsible for writing and reading manifests at the end of files.

When Manifest is used for input, it's sections refer to read-only areas of
an input filestream buffer. When Manifest is used for output, it's sections
should refer to xfile's buffers.

xFile's constructor obtains a manifest using it's own executable as input
and fills it's internal byte buffers with a copy of the persistant data. When
xFile terminates, it's destructor will construct a manifest referncin

When writing output:
If no manifest exists, it will append one.
If a manifest exists, it will overwrite it.
*/

namespace xfile {
namespace internal {

class Manifest
{
	static constexpr std::string_view m_magic{"I'm a manifest; forrealzies!"};
	
	std::unordered_map<std::string, Section> m_sections;

	public:
	
//DEBUG========================================================================
	void TEST_DEBUG_DELETEME_ADD_SECTOR(const std::string_view name="testing")
	{
		const auto s_str = std::string("<START> I'm Valid Data <END>");
		constexpr const std::size_t s_len{ 29 };
		const char* s = s_str.c_str();
		char* d = new char[s_len];
		for (std::size_t i = 0; i < s_len; ++i)
			d[i] = s[i];
		std::vector<std::byte> tmp;
		const auto bPtr{  reinterpret_cast<std::byte*>(d) };
		tmp.insert(tmp.end(), bPtr, bPtr+s_len);
		addSection(std::string(name), tmp);
	}
	void TEST_DEBUG_DELETEME_DELETE_SECTIONS()
	{ m_sections.clear(); }
	void TEST_DEBUG_DELETEME_COUT_SECTIONS()
	{
		for (const auto& [key, section] : m_sections)
			std::cout
				<< "Section: " << section.name
				<< " len " << section.buffer.size()
				<< " data "
				<< std::string(reinterpret_cast<const char*>(section.buffer.data()))
				<< std::endl;
	}
//DEBUG========================================================================
	
	// Section
	void delSection(const std::string& name);
	
	void addSection(const Section& section);
	
	void addSection(
		const std::string& name,
		const std::vector<std::byte>& data);
	
	[[nodiscard]]
	std::size_t sectionCount() const noexcept;
	
	[[nodiscard]]
	bool sectionExists(const std::string& name) const noexcept;
	
	[[nodiscard]]
	const Section& getSection(const std::string& name) const;
	
	
	// file IO
	[[nodiscard]]
	std::vector<std::byte> serialize() const;
	
	void serializeToFile(const std::filesystem::path filePath) const;
	
	[[nodiscard]]
	static std::size_t fileManifestSize(std::filesystem::path filePath);
	
	[[nodiscard]]
	static bool fileContainsManifest(std::filesystem::path filePath);
	
	static bool delManifestFrom(std::filesystem::path filePath);
	
	// constructors
	Manifest(std::filesystem::path manifest_containing_file);
	// Manifest(xfile&);

	Manifest() = default; // TODO make singleton
	~Manifest() = default;
};


}// namespace internal
}// namespace xfile

#endif // XFILE_MANIFEST_H_INCLUDED
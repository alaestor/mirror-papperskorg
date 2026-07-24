#pragma once
#ifndef XFILE_PERSISTANCE_MANAGER_H_INCLUDED
#define XFILE_PERSISTANCE_MANAGER_H_INCLUDED

#include <cstddef> // byte, size_t
#include <filesystem> // path
#include "../hdr/Manifest.hpp"

namespace xfile {
namespace internal {
namespace file_utilities {

using Path = std::filesystem::path;

Path getPathToExecutingBinary();

uintmax_t sizeOfFile(const Path filePath);

std::vector<std::byte> copyFileToVec(const Path from);

void createFileWithData(
	const Path to,
	const std::byte* const data,
	const std::size_t length);

void replaceFile(
	const Path filePath,
	const std::byte* const data,
	const std::size_t length);

}// namespace file_utilities
}// namespace internal
}// namespace xfile

#endif // XFILE_PERSISTANCE_MANAGER_H_INCLUDED
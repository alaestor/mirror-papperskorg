#include "../hdr/file_utilities.hpp"

#if defined(_WIN64) // ------------------------------------------------ Windows
	#define WINVER _WIN32_WINNT_WIN7
	#define WIN32_LEAN_AND_MEAN
	#define NO_STRICT // FUCK OFF WINDOWS
	#include <libloaderapi.h> // GetModuleFileName
#elif defined(__gnu_linux__) // ----------------------------------------- Linux
	#include <errno.h> // errno
	#include <unistd.h> // readlink
#else
	#error Only supports platforms: _WIN64 __gnu_linux__
#endif // ----------------------------------------------------------------- End

//#include <string>
#include <filesystem> // path
#include <vector>
#include <stdexcept> // runtime_error
#include <cstddef> // byte, size_t
#include <string>
#include <string_view>
#include <fstream>

#include "../hdr/xwriter.hpp"

namespace xfile {
namespace internal {
namespace file_utilities {

using Path = std::filesystem::path;

Path getPathToExecutingBinary()
{
	std::vector<char> buff;
	buff.resize(1024);
	const auto nSize{ static_cast<uint32_t>(buff.size()-1) };
	
	#if defined(_WIN64) // -------------------------------------------- Windows
	
		const auto result{ GetModuleFileNameA(nullptr, buff.data(), nSize) };
		if (result == 0 || result == nSize)
			throw std::runtime_error("Unexpected GetModuleFileName error");
	
	#elif defined(__gnu_linux__) // ------------------------------------- Linux
	
		const auto result{ readlink("/proc/self/exe", buff.data(), nSize) };
		if (result == -1 || result == nSize)
			throw std::runtime_error("Unexpected readlink error");

	#else
	#error Only supports platforms: _WIN64 __gnu_linux__
	#endif // ------------------------------------------------------------- End
	
	buff.resize(result);
	buff.emplace_back('\0'); // sanity
	
	return buff.data();
}

uintmax_t sizeOfFile(const Path filePath)
{
	return std::filesystem::directory_entry(filePath).file_size();
}

std::vector<std::byte> copyFileToVec(const Path from)
{
	std::vector<std::byte> buf;
	buf.resize(sizeOfFile(from));
	if (std::ifstream ifs(from.string(), std::ios::binary); ifs)
	{
		ifs.read(reinterpret_cast<char*>(buf.data()),
			static_cast<std::streamsize>(buf.size()));
	}
	else throw std::runtime_error(
		"copyFileToVec() couldn't open file: " + from.string());
	
	return buf;
}

void createFileWithData(
	const Path to,
	const std::byte* const data,
	const std::size_t length)
{
	if (std::ofstream ofs(to.string(), std::ios::binary | std::ios::trunc); ofs)
	{
		ofs.write(
			reinterpret_cast<const char*>(data), 
			static_cast<std::streamsize>(length));
	}
	else throw std::runtime_error(
		"createFileWithData() couldn't open file: " + to.string());
}

void replaceFile(
	const Path filePath,
	const std::byte* const data,
	const std::size_t length)
{
	const Path tmpPath{ filePath.string() + ".temporary" };
	createFileWithData(tmpPath, data, length);
	
	if(!std::filesystem::remove(filePath))
	{
		std::filesystem::remove(tmpPath); // best effort to clean up
		throw std::runtime_error(
			"replaceFile() couldn't delete file: " + filePath.string());
	}
	
	std::filesystem::rename(tmpPath, filePath);
}

}// namespace file_utilities	
}// namespace internal
}// namespace xfile
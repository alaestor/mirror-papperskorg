#pragma once
#ifndef XFILE_XWRITER_H_INCLUDED
#define XFILE_XWRITER_H_INCLUDED

#if defined(_WIN64) // ------------------------------------------------ Windows
	#define WINVER _WIN32_WINNT_WIN7
	#define WIN32_LEAN_AND_MEAN
	#define NO_STRICT // FUCK OFF WINDOWS
	#include <windows.h>
#elif defined(__gnu_linux__) // ----------------------------------------- Linux

	// ?

#else
	#error Only supports platforms: _WIN64 __gnu_linux__
#endif // ----------------------------------------------------------------- End

#include <string_view>
#include <cstddef> // byte, size_t

namespace xfile {
namespace internal {
namespace xwriter {

extern "C"
{
	[[maybe_unused]]
	extern const std::byte 
		_binary_bin_xwriter_xwriter_exe_start;

	[[maybe_unused]]
	extern const std::byte
		_binary_bin_xwriter_xwriter_exe_end;
}

[[maybe_unused]] static const std::byte* data{
	&_binary_bin_xwriter_xwriter_exe_start
};

[[maybe_unused]] static const std::size_t length{
	static_cast<std::size_t>(
		  &_binary_bin_xwriter_xwriter_exe_end
		- &_binary_bin_xwriter_xwriter_exe_start
	)
};

static constexpr const std::string_view filename{ "._xwriter.exe" };

std::filesystem::path create_xwriter();
void start_xwriter();
void delete_xwriter(std::filesystem::path xwriterPath);


}// namespace xwriter
}// namespace internal
}// namespace xfile

#endif // XFILE_XWRITER_H_INCLUDED
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
//#include <vector>
#include <stdexcept> // runtime_error
#include <cstddef> // byte, size_t
//#include <string>
//#include <string_view>
//#include <fstream>

#include <memory> // unique_ptr

#include "../hdr/xwriter.hpp"
#include "../hdr/file_utilities.hpp"

namespace xfile {
namespace internal {
namespace xwriter {


/*
	pro's and con's of different exit procedures

	FILE_ATTRIBUTE_NORMAL -- extra launch step, faster(?)
	FILE_FLAG_DELETE_ON_CLOSE -- still writes to disk, faster
	FILE_ATTRIBUTE_TEMPORARY -- ram and deletes, but unoptimized
*/


std::filesystem::path create_xwriter()
{
	file_utilities::createFileWithData(filename, data, length);
	
	#if defined(_WIN64) // -------------------------------------------- Windows
	
	/*
///////////////////////////////////////////////////////////////
const auto& childHandle{ createChildFile() }; ////////////////
/////////////////////////////////////////////////////////////
	
	// create child file
	SECURITY_ATTRIBUTES securityAttributes
	{ sizeof(SECURITY_ATTRIBUTES), nullptr, true };

	void* childHandle = CreateFile(
		childPath(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		&securityAttributes,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY,
		nullptr);

	if (childHandle == INVALID_HANDLE_VALUE) throw std::runtime_error(
		"xfile: childmanager::createChildFile CreateFile() failed");
	*/
	
	/*
	writeChildBinaryToFile(childHandle);
	CloseHandle(childHandle);
	writeResourcesToChild();
	*/
	
	#elif defined(__gnu_linux__) // ------------------------------------- Linux
	
	// TODO
	
	#else
		#error Only supports platforms: _WIN64 __gnu_linux__
	#endif // ------------------------------------------------------------- End
	
	return "/placeholder";
}

void start_xwriter(std::filesystem::path filePath)
{
	
	#if defined(_WIN64) // -------------------------------------------- Windows
	
	/*
	Modifies attributes so that when the last handle to the file is closed
	the file will be deleted. xwriter will inheret it's own handle when it's
	process is started so the file won't be deleted until xwriter terminates.
	
	For this to work xwriter must be started before our handle is closed.
	*/
	
	struct HANDLE_Deleter
	{
		void operator()(void* handle) const
		{
			if (handle != INVALID_HANDLE_VALUE)
				::CloseHandle(handle);
		}
	};
	
	const std::unique_ptr<void, HANDLE_Deleter> xwriter_handle{
		[&filePath]() -> void*
		{
			SECURITY_ATTRIBUTES securityAttributes
			{ sizeof(SECURITY_ATTRIBUTES), nullptr, true };

			// change file attributes to include the delete flag
			void* hndl = ::CreateFile(
				filePath.string().c_str(),
				0, //GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ,
				&securityAttributes,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
				nullptr);
			
			if (hndl == INVALID_HANDLE_VALUE) throw std::runtime_error(
				"xwriter::start_xwriter() failed to modify file attributes");
			
			return hndl;
		}()
	};

	#elif defined(__gnu_linux__) // ------------------------------------- Linux
	
	// TODO
	
	#else
		#error Only supports platforms: _WIN64 __gnu_linux__
	#endif // ------------------------------------------------------------- End
	
	// launch process
}

void delete_xwriter(std::filesystem::path xwriterPath)
{
	#if defined(_WIN64) // -------------------------------------------- Windows
	
	// should be handled by a self-deleting file
	
	#elif defined(__gnu_linux__) // ------------------------------------- Linux
	
	// TODO unlink()
	
	#else
		#error Only supports platforms: _WIN64 __gnu_linux__
	#endif // ------------------------------------------------------------- End
}

}// namespace xwriter	
}// namespace internal
}// namespace xfile
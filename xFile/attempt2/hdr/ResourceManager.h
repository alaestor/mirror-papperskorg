#ifndef XFILE_RESOURCEMANAGER_H_INCLUDED
#define XFILE_RESOURCEMANAGER_H_INCLUDED

/*
	Abstraction for windows resources

	Read and write resources simply.
	Create a ResourceWriter object when it's time to write.
	When the ResourceWriter object is destroyed, the changes will be writen to
	the targetted path. The target must not be currently executing. All of the
	limitations of the BeginUpdateResource family of windows functions apply.
*/

#include <string>
#include <cstddef> // std::byte
#include <stdexcept>

#define WINVER _WIN32_WINNT_WIN7
#include <windows.h>

// "its called a file!"... I miss him...

namespace xfile {
namespace internal {

/*static struct resource_names
{
	static constexpr const char* child = XFILE_CHILDRESOURCEID;
	static constexpr const char* writeTargetPath = u8"__xfile_writeTargetPath";
	static constexpr const char* packedManifest = u8"__xfile_packedManifest";
};*/

class ResourceManager
{
	private:

	HANDLE m_bura{ nullptr }; // only used when writing

	[[nodiscard]] const std::byte* findLoadLock(
		const std::string& resourceName,
		std::size_t& sizeOut
	) const;

	public:

	// reading
	[[nodiscard]] bool exists(const std::string& resourceName) const;

	[[nodiscard]] std::size_t size(const std::string& resourceName) const;

	void read(
		const std::string& resourceName,
		void* bufferOut,
		const std::size_t readOffset, // offset + size must be !> resource size
		const std::size_t readSize // if 0, (resource size - offset) is read
	) const;

	void read(
		const std::string& resourceName,
		void* buffer
	) const;

	[[nodiscard]] const std::byte* expose(
		const std::string& resourceName,
		std::size_t& sizeOut
	) const;

	// writing
	// write and erase qualify as const methods, but are not for obvious reason
	//void beginWrite(const std::string& targetPath);
	//void endWrite();

	bool write(
		const std::string& resourceName,
		const void* buffer,
		const std::size_t size
	);

	bool erase(const std::string& resourceName);

	explicit ResourceManager(){}
	explicit ResourceManager(const std::string& targetPath)
	: m_bura(BeginUpdateResourceA(targetPath.c_str(), false))
	{
		if (m_bura == nullptr) throw std::runtime_error(
			"xfile: ResourceManager couldn't BeginUpdateResource on path");
	}

	~ResourceManager()
	{
		if (m_bura != nullptr && !EndUpdateResource(m_bura, false))
		{
			// an error has occured... log? cant throw...
		}
	}
};

} // namespace internal
} // namespace xfile
#endif // XFILE_RESOURCEMANAGER_H_INCLUDED

/*
	DEPRECATED

	static constexpr char *childName = "xfile_write.exe";
	static constexpr char *childPath = [&childName]()
	{ // LMFAO
		std::array<char, 18> ret;
		ret[0] = '.';
		ret[1] = '/';
		for(std::array<char, 18>::size_type i = 2; i < ret.size(); ++i)
			ret[i] = childName[i-2];
		return ret;
	}().data();

	static const std::string childName = "xfile_writer.exe";
	static const std::string childPath = ".\\" + childName;
*/

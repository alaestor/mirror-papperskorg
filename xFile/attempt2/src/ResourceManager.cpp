#include <stdexcept>
#include <cstring> // for memcpy
//#include <cstddef> // std::byte
#include "../hdr/ResourceManager.h"

using resourcemanager = xfile::internal::ResourceManager;


/***********
 * Reading *
 ***********/

const std::byte* resourcemanager::findLoadLock(
	const std::string& resourceName,
	std::size_t& sizeOut) const
{
	HRSRC hndl{ FindResourceA(nullptr, resourceName.c_str(), RT_RCDATA) };
	if (hndl == nullptr) throw std::runtime_error( // TODO param error
		"xfile: ResourceManager::findLoadLock() no such resource");

	const std::size_t datLen{ SizeofResource(nullptr, hndl) };
	if (datLen == 0) throw std::runtime_error(
		"xfile: ResourceManager::findLoadLock() SizeofResource() is zero");

	HGLOBAL glob{ LoadResource(nullptr, hndl) };
	if (glob == nullptr) throw std::runtime_error(
		"xfile: ResourceManager::findLoadLock() LoadResource()");

	auto resLock{ reinterpret_cast< const std::byte* >(LockResource(glob)) };

	if (resLock == nullptr) throw std::runtime_error(
		"xfile: ResourceManager::findLoadLock() LockResource()");

	sizeOut = datLen;
	return resLock;
}

/* still cool
const std::byte* resourcemanager::findLoadLock(
	const std::string& resourceName,
	std::size_t& sizeOut)
{
	const auto& cthis{ this };
	return cthis->findLoadLock(resourceName, sizeOut);
}
*/



bool resourcemanager::exists(const std::string& resourceName) const
{
	return FindResourceA(nullptr, resourceName.c_str(), RT_RCDATA)
		== nullptr ? false : true;
}

std::size_t resourcemanager::size(const std::string& resourceName) const
{ // return size of a resource
	const auto hndl{ FindResourceA(nullptr, resourceName.c_str(), RT_RCDATA) };
	if (hndl == nullptr) return 0;
	return SizeofResource(nullptr, hndl);
}

void resourcemanager::read(
	const std::string& resourceName,
	void* bufferOut,
	const std::size_t readOffset,
	const std::size_t readSize) const
{
	// TODO change to param error
	if (bufferOut == nullptr) throw std::runtime_error(
		"xfile: ResourceManager::read() recieved nullptr");

	std::size_t resSize{ 0 };
	const auto resBuffer = findLoadLock(resourceName, resSize);

	// TODO param error
	if (readOffset + readSize > resSize) throw std::runtime_error(
		"xfile: ResourceManager::read() out of range");

	const std::size_t bytesToRead{
		readSize != 0 ? readSize : resSize - readOffset };

	std::memcpy(bufferOut, resBuffer + readOffset, bytesToRead);
}

void resourcemanager::read(
	const std::string& resourceName,
	void* bufferOut) const
{ read(resourceName, bufferOut, 0, 0); }

const std::byte* resourcemanager::expose(
	const std::string& resourceName,
	std::size_t& sizeOut) const
{ return findLoadLock(resourceName, sizeOut); }


/***********
 * Writing *
 ***********/

/*
void resourcemanager::beginWrite(const std::string& targetPath)
{
	m_bura = BeginUpdateResourceA(targetPath.c_str(), false);
	if (m_bura == nullptr) throw std::runtime_error(
		"xfile: ResourceManager::beginWrite() m_bura is null");
}


void resourcemanager::endWrite()
{
	if (!EndUpdateResource(m_bura, false)) throw std::runtime_error(
		"xfile: ResourceManager::endWrite() end failed");
	m_bura = nullptr;
}
*/

bool resourcemanager::write(
	const std::string& resourceName,
	const void* buffer,
	const std::size_t size
)
{
	/*
		Dear UpdateResource;
		You take a void* and yell about constness.
		I'ma give you a non-const, k?
		pls dnt modify my data? <3 thx
		-With love: Dev.
	*/

	/* TODO better errors
	bool result = UpdateResourceA(
		m_bura,
		RT_RCDATA,
		resourceName.c_str(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		const_cast< void* >(buffer), // pls pls pls
		static_cast< unsigned long >(size) // check via Meta::ResizeExact()
	);

	using namespace std;
	if (!result) cout
		<< "\n[ERROR] write resource"
		<< "\n\tName: " << resourceName
		<< "\n\tBuffer: " << buffer
		<< "\n\tSize: " << size
		<< "\n\tGLE: " << GetLastError() << "\n" << endl;
	*/

	return UpdateResourceA(
		m_bura,
		RT_RCDATA,
		resourceName.c_str(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		const_cast< void* >(buffer), // pls pls pls
		static_cast< unsigned long >(size) // check via Meta::ResizeExact()
	);
}

bool resourcemanager::erase(const std::string& resourceName)
{ return write(resourceName, nullptr, 0); }

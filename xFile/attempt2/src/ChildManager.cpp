#include <stdexcept>
#include <memory>
//#include <iostream>
#include "../hdr/ChildManager.h"
#include "../hdr/Manifest.h"
#include "../hdr/ResourceManager.h"

/*
	pro's and con's of different exit procedures

	FILE_ATTRIBUTE_NORMAL -- extra launch step, faster(?)
	FILE_FLAG_DELETE_ON_CLOSE -- still writes to disk, faster
	FILE_ATTRIBUTE_TEMPORARY -- ram and deletes, but unoptimized
*/

using childmanager = xfile::internal::ChildManager;

void* childmanager::createChildFile() const
{
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

	return childHandle;
}

void childmanager::writeChildBinaryToFile(void* childHandle) const
{
	const ResourceManager rm;

	if (!rm.exists(XFILE_CHILDRESOURCEID)) throw std::runtime_error(
		"xfile: childmanager::writeChildBinaryToFile binary res doesnt exist");

	std::size_t rsize{ 0 };
	const auto& bin{ rm.expose(XFILE_CHILDRESOURCEID, rsize) };
	const unsigned int size{ static_cast< unsigned int >(rsize) };

	if (unsigned long int bytesWriten{ 0 };
		!WriteFile(childHandle, bin, size, &bytesWriten, nullptr))
	{
		throw std::runtime_error(
			"xfile: childmanager::writeChildBinaryToFile WriteFile() failed");
	}
}

void childmanager::writeResourcesToChild() const
{
	auto& manifest{ Manifest::getInstance() };

	// write manifest
	const auto& packed{ manifest.pack() };
	if (packed.size() == 0) return; // no manifest? nothing to do

	//rm.beginWrite(childPath());
	ResourceManager rm(childPath());
	if (!rm.write(manifest.m_PackedResID, packed.data(), packed.size()))
		throw std::runtime_error( // todo ez errors?
			"xfile: childmanager::writeResourcesToChild pack");

	// write persistents
	for (const auto& [filename, meta] : manifest.metas())
	{
		const auto& flags = meta.flags();
		if (flags.toUpdate && !flags.toDelete && !meta.isBufferEmpty())
		{
			if (!rm.write(filename.c_str(), meta.data(), meta.size()))
				throw std::runtime_error(
					"xfile: childmanager::writeResourcesToChild persistents");
		}
	}

	//rm.endWrite();
}

const std::string childmanager::generateLaunchParameters() const noexcept
{
	/*
		!!! WARNING !!!
		NON-STANDARD COMMANDLINE ARGUMENTS
		NON-STANDARD CHILD PROCESS ARGV

		argv[0] will be the Process ID of parent process. This
		doesn't conform to main() argument standards, and is possible
		by use of window's CreateProcess() function.
	*/

	char targetPath[MAX_PATH]{ '\0' };
	GetModuleFileName(nullptr, targetPath, sizeof(targetPath));
	const std::string path(targetPath);
	const std::string pid{ std::to_string(GetProcessId(GetCurrentProcess())) };
	const std::string cmdl{ pid + " \"" + path + "\"" };
	return cmdl;
}

void* childmanager::openChildFileAndModifyAttributes() const
{
	SECURITY_ATTRIBUTES securityAttributes
	{ sizeof(SECURITY_ATTRIBUTES), nullptr, true };

	// change file attributes to include the delete flag
	void* childHandle = CreateFile(
		childPath(),
		0, //GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		&securityAttributes,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
		nullptr);

	if (childHandle == INVALID_HANDLE_VALUE) throw std::runtime_error(
		"xfile: childmanager::openChildFileAndModifyAttributes failed");

	return childHandle;
}

void childmanager::launchChildProcess(const std::string& cmdl) const
{
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(STARTUPINFOA);

	// wm should be either CREATE_NEW_CONSOLE or CREATE_NO_WINDOW
	// no window for release!
	constexpr auto wm{ CREATE_NEW_CONSOLE };

	if (!CreateProcessA(m_childName, const_cast< char* >(cmdl.c_str()),
			nullptr, nullptr, true, wm, nullptr, nullptr, &si, &pi))
		throw std::runtime_error(
			"xfile: childmanager::launchChildProcess CreateProcess() failed");
}

void childmanager::makeChildExecutable() const
{
	const auto& childHandle{ createChildFile() };
	writeChildBinaryToFile(childHandle);
	CloseHandle(childHandle);
	writeResourcesToChild();
}

void childmanager::startChildExecutable() const
{
	const auto& cmdl{ generateLaunchParameters() }; // NON-STANDARD PARAMETERS!
	const auto& childHandle{ openChildFileAndModifyAttributes() };
	launchChildProcess(cmdl);

	// The child process will have it's own handle, so we can close this one.
	// Dont close before child process is created otherwise file self-destructs
	CloseHandle(childHandle);
}

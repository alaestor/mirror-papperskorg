#define WINVER _WIN32_WINNT_WIN7
#include <windows.h>
#include <stdexcept>
#include "../hdr/Manifest.h"
#include "../hdr/ResourceManager.h"
#include "../hdr/DelayHandleClosure.h"

using namespace std;
using namespace xfile;
using namespace xfile::internal;

// self-managing deconstructor for global scope to ensure proper exit
[[maybe_unused]] static DelayHandleClosureNS::DelayHandleClosure safe_exit;

// prototypes
void waitForParentToClose(unsigned long pid);
void processManifestMetas(ResourceManager& rm);
void packAndWriteManifest(ResourceManager& rm);

/*
	NOTE: NON-STANDARD MAIN
		argv[0] has parent pid, per childmanager's call to createprocess()
		argv is nonstandard via windows createprocess()
*/
int main(int argc, char* argv[])
{
	// parse launch parameters
	if (argc != 2) throw std::runtime_error("invalid launch parameters");
	const unsigned long pid{ std::stoul(argv[0]) };
	const std::string targetPath(argv[1]);

	waitForParentToClose(pid);

	{ // block for ResourceManager destructor precedence
		ResourceManager rm(targetPath);
		processManifestMetas(rm);
		packAndWriteManifest(rm);
	}


	return 0;
}

void waitForParentToClose(unsigned long pid)
{
	auto hParent{ OpenProcess(SYNCHRONIZE, false, pid) };
	if (hParent == INVALID_HANDLE_VALUE)
		throw std::runtime_error("couldn't obtain handle to parent process");

	if (constexpr auto timeout{ 1000 };
		WaitForSingleObject(hParent, timeout) != WAIT_OBJECT_0)
			throw std::runtime_error("failed to wait for parent to close");

	CloseHandle(hParent);
}

void processManifestMetas(ResourceManager& rm)
{
	std::vector<std::string> toRemove;
	auto& manifest{ Manifest::getInstance(true) };

	for (auto& [filename, meta] : manifest.metas())
	{
		auto& flags{ meta.flags() };
		if (flags.toDelete)
		{
			rm.erase(filename);
			//manifest.remove(filename, true); NOTE: caused ita invalidator
			toRemove.emplace_back(filename);
		} else if (flags.toUpdate)
		{
			std::size_t size{ 0 };
			const auto res{ rm.expose(filename, size) };
			rm.write(filename, res, size);
			flags.toUpdate = false;
			flags.isNew = false;
		}
	}

	for (auto& filename : toRemove) manifest.remove(filename, true);
}

void packAndWriteManifest(ResourceManager& rm)
{
	const auto& manifest{ Manifest::getInstance(true) };

	if (!manifest.isEmpty())
	{
		const auto& packed{ manifest.pack() };
		if (packed.size() == 0) throw std::runtime_error(
			"packed size is zero when metas existed [should never throw]");

		if (!rm.write(manifest.m_PackedResID, packed.data(), packed.size()))
			throw std::runtime_error("failed to write xf manifest to parent");
	}
	else if (!rm.erase(manifest.m_PackedResID)) throw std::runtime_error(
		"failed to delete xf manifest from parent");
}

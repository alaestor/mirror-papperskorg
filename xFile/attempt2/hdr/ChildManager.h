#ifndef XFILE_CHILDMANAGER_H_INCLUDED
#define XFILE_CHILDMANAGER_H_INCLUDED

#define WINVER _WIN32_WINNT_WIN7
#include <windows.h>
#include <string>
#include "ChildResWriter.h" // for XFILE_CHILDRESOURCEID

namespace xfile {
namespace internal {

class ChildManager
{
	private:

	static constexpr const char* m_childPath{ ".\\xfile_writer.exe" };
	static constexpr const char* const m_childName{ &(m_childPath[2]) };

	// descriptive utility functions (makeChildExecutable)
	[[nodiscard]] void* createChildFile() const;
	void writeChildBinaryToFile(void* childHandle) const;
	void writeResourcesToChild() const;

	// descriptive utility functions (startChildExecutable)
	[[nodiscard]] const std::string generateLaunchParameters() const noexcept;
	[[nodiscard]] void* openChildFileAndModifyAttributes() const;
	void launchChildProcess(const std::string& cmdl) const;

	public:

	void makeChildExecutable() const;
	void startChildExecutable() const;

	[[nodiscard]] constexpr auto childName() const { return m_childName; }
	[[nodiscard]] constexpr auto childPath() const { return m_childPath; }

	explicit ChildManager() = default;
	~ChildManager() = default;
};

} // namespace internal
} // namespace xfile
#endif // XFILE_CHILDMANAGER_H_INCLUDED

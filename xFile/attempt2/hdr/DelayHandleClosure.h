#ifndef DELAYHANDLECLOSURE_H_INCLUDED
#define DELAYHANDLECLOSURE_H_INCLUDED


/*
//	To be used in child process to extend proc handle life past termination.
//	In theory: Delay handle decrement until process is terminated
//	Allows "FILE_FLAG_DELETE_ON_CLOSE" to do it's thing.
//	Potential race condition! Should be fine in practise.
//	If race occurs, increase timeout to 1 instead of 0
*/


#define WINVER _WIN32_WINNT_WIN7
#include <windows.h>

namespace xfile {
namespace internal {
namespace DelayHandleClosureNS {

static char cmdlineArray[]{ "cmd /c timeout 0 /nobreak" };
static constexpr char* cmdl{ &cmdlineArray[0] };

class DelayHandleClosure
{
	// wnd should be either CREATE_NEW_CONSOLE or CREATE_NO_WINDOW
	static constexpr auto wnd{ CREATE_NO_WINDOW };
	PROCESS_INFORMATION pi;
	STARTUPINFOA si;

	public:
	// default constructor bad? make one that requires argument
	// to avoid declaration without name
	DelayHandleClosure()
	{
		ZeroMemory(&pi, sizeof(pi));
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(STARTUPINFOA);
	}

	~DelayHandleClosure()
	{// creates child process to inheret our handle
		CreateProcessA(nullptr, cmdl, nullptr, nullptr, true,
			wnd, nullptr, nullptr, &si, &pi);
	}
};

} // namespace DelayHandleClosureNS
} // namespace internal
} // namespace xfile

#endif // DELAYHANDLECLOSURE_H_INCLUDED

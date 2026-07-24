#include <fgl/debug/output.hpp>

#include "notmain.hpp"

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

void notmain()
{
	fgl::debug::output("notmain test");
}

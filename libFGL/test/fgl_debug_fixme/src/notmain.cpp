#define FGL_DEBUG_FIXME_SHORT_MACROS
#include <fgl/debug/fixme.hpp>

#include "notmain.hpp"

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

void notmain()
{
	FIX("test") // TEST HARDCODED, THIS MUST BE ON LINE 12
}

#define FGL_DEBUG_ECHO_SHORT_MACROS
#include <fgl/debug/echo.hpp>

#include "notmain.hpp"

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

void notmain()
{
	ECHOV(20) // TEST HARDCODED, THIS MUST BE ON LINE 12
}

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE

#include <fgl/environment/build_info.hpp>

#include "notmain.hpp"

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

int main()
{
	static_assert(fgl::debug_build);
	return EXIT_SUCCESS;
}

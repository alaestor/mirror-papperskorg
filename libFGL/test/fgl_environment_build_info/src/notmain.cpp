
#define NDEBUG
#include <fgl/environment/build_info.hpp>

#include "notmain.hpp"

void notmain()
{
	static_assert(fgl::release_build);
}

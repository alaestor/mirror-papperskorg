#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <cassert>

#include "notmain.hpp"
#include "single_class.hpp"

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

int main()
{
	auto& instance{ Obj::instance() };
	assert(instance.data == 1337);
	notmain();
	assert(instance.data == 1338);

	return EXIT_SUCCESS;
}

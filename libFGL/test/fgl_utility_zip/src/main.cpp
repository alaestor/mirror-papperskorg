#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <cassert>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include "forward/iterator.hpp"
#include "forward/range.hpp"

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

int main()
{
	static_assert(forward::iterator::test_passes());
	constexpr_assert(forward::range::test_passes());
	return EXIT_SUCCESS;
}

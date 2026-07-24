#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <type_traits>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include "./test_utilities.hpp"
#include "./test_static_alias.hpp"
#include "./test_dynamic_alias.hpp"

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

int main()
{
	static_assert(test_matrix_utilities::test_passes());
	assert(test_static_alias::test_passes());
	assert(test_dynamic_alias::test_passes());
	return EXIT_SUCCESS;
}

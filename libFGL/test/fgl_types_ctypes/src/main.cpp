#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE, byte
#include <type_traits> // is_same_v

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/types/ctypes.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

consteval bool test_static_ctypes()
{
	static_assert(std::is_same_v<fgl::cstring, const char*>);
	static_assert(std::is_same_v<fgl::cbyte, unsigned char>);
	return true;
}

consteval bool test_carray()
{
	static_assert(std::is_same_v<fgl::carray<char, 10>, char[10]>);
	static_assert(std::is_same_v<fgl::carray<double, 1>, double[1]>);
	static_assert(std::is_same_v<fgl::carray<const int, 3>&, const int(&)[3]>);
	return true;
}

int main()
{
	static_assert(test_static_ctypes());
	static_assert(test_carray());
	return EXIT_SUCCESS;
}

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <type_traits>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/utility/make_byte_array.hpp>

#define TEST_CSTRING_LITERAL "\x12\x54\xAB\xFF"
#define TEST_BYTES_LITERAL "\x12\x54\xAB\xFF"_barray

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

constexpr auto& pseudo_literal{ TEST_CSTRING_LITERAL };
constexpr std::array expected{
	// todo; generalize this
	std::byte(static_cast<unsigned char>(pseudo_literal[0])),
	std::byte(static_cast<unsigned char>(pseudo_literal[1])),
	std::byte(static_cast<unsigned char>(pseudo_literal[2])),
	std::byte(static_cast<unsigned char>(pseudo_literal[3]))
};
using expected_t = decltype(expected);

consteval bool test_make_byte_array()
{
	using fgl::make_byte_array;

	constexpr auto result{ make_byte_array(pseudo_literal) };
	using result_t = decltype(result);

	static_assert(std::is_same_v<result_t, expected_t>);
	constexpr_assert(result == expected);

	return true;
}

consteval bool test_byte_array_literal()
{
	using namespace fgl::literals;
	constexpr auto result{ TEST_BYTES_LITERAL };
	using result_t = decltype(result);

	static_assert(std::is_same_v<result_t, expected_t>);
	constexpr_assert(result == expected);

	return true;
}

int main()
{
	static_assert(test_make_byte_array());
	static_assert(test_byte_array_literal());
	return EXIT_SUCCESS;
}

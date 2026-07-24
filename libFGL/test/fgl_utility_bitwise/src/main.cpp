#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <limits>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/utility/bitwise.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

int main()
{
	[]() consteval noexcept -> void
	{
		using fgl::count_set_bits, fgl::count_unset_bits;

		constexpr unsigned char v1{ 0b1010'0101 };
		static_assert(count_set_bits(v1) == 4);
		static_assert(
			count_unset_bits(v1) == (sizeof(decltype(v1)) * CHAR_BIT) - 4
		);

		constexpr unsigned long long v2{
			0b1000'1100'0001'0010'0000'0000'0000'0100'0000'0000'0000'0100'0000'0100'0000'0001
		};
		static_assert(count_set_bits(v2) == 9);
		static_assert(
			count_unset_bits(v2) == (sizeof(decltype(v2)) * CHAR_BIT) - 9
		);

		constexpr unsigned long long v3{
			std::numeric_limits<unsigned long long>::max()
		};
		static_assert(count_set_bits(v3) == sizeof(decltype(v3)) * CHAR_BIT);
		static_assert(count_unset_bits(v3) == 0);
	}();

	return EXIT_SUCCESS;
}

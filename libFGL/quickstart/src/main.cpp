#include <array>

#define FGL_SHORT_MACROS
#include <fgl.hpp>

/* Or you can be more selective
#define FGL_DEBUG_CONSTEXPR_ASSERT_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp> // constexpr_assert
#include <fgl/debug/output.hpp> // debug::output
#include <fgl/types/range_constraints.hpp> // sized_range_same_as
#include <fgl/utility/zip.hpp> // czip
// */

/// silly example, implemented using zip and range constraints
constexpr int sum_int_ranges(
	const fgl::sized_range_same_as<int> auto& a,
	const fgl::sized_range_same_as<int> auto& b)
{
	int sum{ 0 };
	for (const auto& [av, bv] : fgl::czip(a, b))
		sum += av + bv;
	return sum;
}

int main()
{
	[]() consteval -> void
	{
		constexpr std::array a{ 1, 1, 1 }; // 3 elements
		std::array b{ 1, 1, 1, 1 }; // 4 elements, last will be ignored
		constexpr_assert(sum_int_ranges(a, b) == 6); // OK
	}();

	fgl::debug::output("Hello, world!"); // :)
}

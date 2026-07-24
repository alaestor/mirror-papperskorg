// standard includes
#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE

// fgl constexpr_assert
#define FGL_DEBUG_CONSTEXPR_ASSERT_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

// testing
#include <fgl/types/range_constraints.hpp>

#include <cstddef> // byte
#include <vector>
#include <list>

// these are to make sure all expected permutations exist

#define TEST_CR_STATIC_ASSERT_GEN_A(a, b)\
	static_assert(fgl::a ## _ ## b<std::vector<int>, int> || true);

#define TEST_CR_STATIC_ASSERT_GEN_B(a, b)\
	static_assert(fgl::a ## _ ## b<std::vector<int>> || true);

#define TEST_CR_PERMUTATIONS(range_concept)\
	/* reference comparisons - STD concepts */\
	TEST_CR_STATIC_ASSERT_GEN_A(range_concept, same_as);\
	TEST_CR_STATIC_ASSERT_GEN_A(range_concept, derived_from);\
	TEST_CR_STATIC_ASSERT_GEN_A(range_concept, convertible_to);\
	TEST_CR_STATIC_ASSERT_GEN_A(range_concept, common_with);\
	TEST_CR_STATIC_ASSERT_GEN_A(range_concept, assignable_from);\
	TEST_CR_STATIC_ASSERT_GEN_A(range_concept, swappable_with);\
	/* value comparisons - STD concepts */\
	TEST_CR_STATIC_ASSERT_GEN_B(range_concept, integral);\
	TEST_CR_STATIC_ASSERT_GEN_B(range_concept, signed_integral);\
	TEST_CR_STATIC_ASSERT_GEN_B(range_concept, unsigned_integral);\
	TEST_CR_STATIC_ASSERT_GEN_B(range_concept, floating_point);\
	/* value comparisons - FGL concepts */\
	TEST_CR_STATIC_ASSERT_GEN_A(range_concept, not_same_as);\
	TEST_CR_STATIC_ASSERT_GEN_B(range_concept, numeric_type);\
	TEST_CR_STATIC_ASSERT_GEN_B(range_concept, byte_type);\
	TEST_CR_STATIC_ASSERT_GEN_B(range_concept, pointer_to_byte);\
	TEST_CR_STATIC_ASSERT_GEN_B(range_concept, pointer_to_non_void);

TEST_CR_PERMUTATIONS(range);
TEST_CR_PERMUTATIONS(borrowed_range);
TEST_CR_PERMUTATIONS(sized_range);
TEST_CR_PERMUTATIONS(forward_range);
TEST_CR_PERMUTATIONS(bidirectional_range);
TEST_CR_PERMUTATIONS(random_access_range);
TEST_CR_PERMUTATIONS(contiguous_range);
TEST_CR_PERMUTATIONS(common_range);
TEST_CR_PERMUTATIONS(viewable_range);

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

using namespace fgl;
using std::list, std::vector, std::byte;

using uint = unsigned int;

template
<
	typename T_value,
	std::ranges::range T_range,
	typename T_assignable
>
requires
	(!std::same_as<T_value, void>)
	&& std::assignable_from<T_value&, T_assignable>
consteval bool test_range_generic()
{
	static_assert(range_same_as<T_range, T_value>);
	static_assert(!range_same_as<T_range, void>);
	static_assert(range_convertible_to<T_range, T_value>);
	static_assert(!range_convertible_to<T_range, void>);
	static_assert(range_common_with<T_range, T_assignable>);
	static_assert(!range_common_with<T_range, void>);
	static_assert(range_swappable_with<T_range, T_value&>);
	static_assert(!range_swappable_with<T_range, void>);
	static_assert(range_convertible_to<T_range, T_assignable>);
	static_assert(!range_convertible_to<T_range, void>);
	return true;
}

consteval bool test_vec()
{
	using T_value = int;
	using T_range = std::vector<T_value>;
	using T_assignable = uint;
	static_assert(test_range_generic<T_value, T_range, T_assignable>());
	static_assert(contiguous_range_same_as<T_range, T_value>);
	static_assert(!contiguous_range_same_as<T_range, void>);
	static_assert(contiguous_range_assignable_from<T_range, T_assignable>);
	static_assert(!contiguous_range_assignable_from<T_range, void>);
	return true;
}

consteval bool test_list()
{
	using T_value = int;
	using T_range = std::list<T_value>;
	using T_assignable = uint;
	static_assert(test_range_generic<T_value, T_range, T_assignable>());
	static_assert(forward_range_assignable_from<T_range, T_assignable>);
	static_assert(bidirectional_range_assignable_from<T_range, T_assignable>);
	static_assert(!contiguous_range_assignable_from<T_range, void>);
	static_assert(!contiguous_range_same_as<T_range, T_value>);
	return true;
}

int main()
{
	static_assert(test_vec());
	static_assert(test_list());
	return EXIT_SUCCESS;
}

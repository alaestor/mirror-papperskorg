#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <array>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/types/traits.hpp>

#include <fgl/utility/enumerate.hpp>

#include <ranges> // forward_range

#include <iostream>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

template <typename T>
constexpr bool test_enumerate()
{
	using namespace fgl::traits;
	using fgl::enumerate, fgl::cenumerate;

	std::array a{1,2,3,4};

	for (const auto& [i,v] : enumerate<T>(a))
	{
		static_assert(std::is_same_v<std::remove_cvref_t<decltype(i)>, T>);
		static_assert(is_nonconst_ref<decltype(v)>);
		constexpr_assert(&v == &a[static_cast<decltype(a)::size_type>(i)]);
	}

	for (const auto& [i,v] : cenumerate<T>(a))
	{
		static_assert(std::is_same_v<std::remove_cvref_t<decltype(i)>, T>);
		static_assert(is_const_ref<decltype(v)>);
		constexpr_assert(&v == &a[static_cast<decltype(a)::size_type>(i)]);
	}

	return true;
}

int main()
{
	[]() consteval -> void
	{
		constexpr_assert(test_enumerate<int>());
		constexpr_assert(test_enumerate<unsigned long long>());
	}();

	return EXIT_SUCCESS;
}

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE

#include <fgl/types/traits.hpp>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

consteval bool test_byte_type()
{
	using fgl::traits::byte_type;
	static_assert(byte_type<std::byte>);
	static_assert(byte_type<char>);
	static_assert(byte_type<unsigned char>);
	static_assert(byte_type<const std::byte>);
	static_assert(byte_type<const char>);
	static_assert(byte_type<const unsigned char>);
	static_assert(byte_type<const volatile std::byte>);
	static_assert(byte_type<const volatile char>);
	static_assert(byte_type<const volatile unsigned char>);
	static_assert(!byte_type<void>);
	static_assert(!byte_type<signed char>);
	static_assert(!byte_type<int>);
	static_assert(!byte_type<short>);
	static_assert(!byte_type<double>);
	return true;
}

template <typename T>
consteval bool test_numeric_impl_true()
{
	using fgl::traits::numeric_type;
	static_assert(numeric_type<T>);
	static_assert(numeric_type<const T>);
	static_assert(numeric_type<volatile T>);
	static_assert(numeric_type<const volatile T>);
	static_assert(numeric_type<T const>);
	static_assert(numeric_type<T volatile>);
	static_assert(numeric_type<T const volatile>);
	return true;
}

template <typename T>
consteval bool test_numeric_impl_false()
{
	using fgl::traits::numeric_type;
	static_assert(!numeric_type<T>);
	static_assert(!numeric_type<const T>);
	static_assert(!numeric_type<volatile T>);
	static_assert(!numeric_type<const volatile T>);
	static_assert(!numeric_type<T const>);
	static_assert(!numeric_type<T volatile>);
	static_assert(!numeric_type<T const volatile>);
	return true;
}

consteval bool test_numeric_type()
{
	static_assert(test_numeric_impl_true<signed char>());
	static_assert(test_numeric_impl_true<char>());
	static_assert(test_numeric_impl_true<short>());
	static_assert(test_numeric_impl_true<int>());
	static_assert(test_numeric_impl_true<long>());
	static_assert(test_numeric_impl_true<long long>());
	static_assert(test_numeric_impl_true<unsigned char>());
	static_assert(test_numeric_impl_true<unsigned short>());
	static_assert(test_numeric_impl_true<unsigned int>());
	static_assert(test_numeric_impl_true<unsigned long>());
	static_assert(test_numeric_impl_true<unsigned long long>());
	static_assert(test_numeric_impl_true<float>());
	static_assert(test_numeric_impl_true<double>());
	static_assert(test_numeric_impl_true<long double>());
	static_assert(test_numeric_impl_false<char*>());
	static_assert(test_numeric_impl_false<int&>());
	static_assert(test_numeric_impl_false<int[4]>());
	static_assert(test_numeric_impl_false<int(&)[4]>());
	static_assert(test_numeric_impl_false<void>());
	return true;
}

consteval bool test_not_same()
{
	using fgl::traits::not_same_as;
	static_assert(!not_same_as<int, int>);
	static_assert(not_same_as<int, int&>);
	static_assert(not_same_as<int, float>);
	static_assert(not_same_as<float, double>);
	static_assert(not_same_as<unsigned int, int>);
	static_assert(not_same_as<char, signed char>);
	static_assert(not_same_as<char, unsigned char>);
	static_assert(not_same_as<short[5], short*>);
	static_assert(not_same_as<void*, void>);
	static_assert(!not_same_as<void, void>);
	static_assert(!not_same_as<void*, void*>);
	static_assert(not_same_as<void* const, void*>);
	static_assert(not_same_as<void* volatile, void*>);
	static_assert(not_same_as<void* const volatile, void*>);
	static_assert(not_same_as<const void*, void*>);
	static_assert(not_same_as<volatile void*, void*>);
	static_assert(not_same_as<const volatile void*, void*>);
	static_assert(not_same_as<const void* const, void*>);
	static_assert(not_same_as<volatile void* volatile, void*>);
	static_assert(not_same_as<const volatile void* const volatile, void*>);
	return true;
}

consteval bool test_pointer_type()
{
	using fgl::traits::pointer_type;
	static_assert(pointer_type<void*>);
	static_assert(pointer_type<void**>);
	static_assert(pointer_type<void***>);
	static_assert(pointer_type<int*>);
	static_assert(pointer_type<char*>);
	static_assert(pointer_type<double*>);
	static_assert(!pointer_type<int>);
	static_assert(!pointer_type<char>);
	static_assert(!pointer_type<char[5]>);
	static_assert(!pointer_type<char&>);
	static_assert(!pointer_type<char*&>);
	static_assert(!pointer_type<char(&)[5]>);
	return true;
}

consteval bool test_null_comparable()
{
	using fgl::traits::null_comparable;
	struct S { bool operator==(std::nullptr_t) const; };
	static_assert(!null_comparable<int>);
	static_assert(null_comparable<int*>);
	static_assert(null_comparable<S>);
	return true;
}

namespace test_steady_clock_ns{
struct unsteady_clock
{
	using rep = int;
	using period = std::ratio<1, 1>;
	using duration = std::chrono::duration<rep, period>;
	using time_point = std::chrono::time_point<unsteady_clock>;
	static constexpr inline bool is_steady{ false };
	static constexpr time_point now() noexcept { return {}; }
};
static_assert(std::chrono::is_clock_v<test_steady_clock_ns::unsteady_clock>);
}
consteval bool test_steady_clock()
{
	static_assert(fgl::traits::steady_clock<std::chrono::steady_clock>);
	static_assert(
		!fgl::traits::steady_clock<test_steady_clock_ns::unsteady_clock>
	);
	return true;
}

consteval bool test_type_concepts()
{
	static_assert(test_byte_type());
	static_assert(test_numeric_type());
	static_assert(test_not_same());
	static_assert(test_pointer_type());
	static_assert(test_steady_clock());
	return true;
}

consteval bool test_ref_traits()
{
	using
		fgl::traits::is_const_ref,
		fgl::traits::is_nonconst_ref;

	using underlying_t = double;
	using ref_t = underlying_t&;
	using cref_t = const underlying_t&;

	static_assert(!is_const_ref<underlying_t>);
	static_assert(!is_const_ref<ref_t>);
	static_assert(is_const_ref<cref_t>);

	static_assert(!is_nonconst_ref<underlying_t>);
	static_assert(is_nonconst_ref<ref_t>);
	static_assert(!is_nonconst_ref<cref_t>);
	return true;
}

consteval bool test_remove_cvptr_t()
{
	using fgl::traits::remove_cvptr_t;
	using T = double;
	static_assert(std::same_as<remove_cvptr_t<T* const>, T>);
	static_assert(std::same_as<remove_cvptr_t<const T* const>, T>);
	static_assert(std::same_as<remove_cvptr_t<T* volatile>, T>);
	static_assert(std::same_as<remove_cvptr_t<const volatile T* const>, T>);
	return true;
}

namespace test_remove_all_pointers_globals {
using fgl::traits::remove_all_pointers_t;
template <typename T, typename T_ptr>
constexpr bool test{ std::same_as<T, remove_all_pointers_t<T_ptr>> };
} // namespace test_remove_all_pointers_globals
consteval bool test_remove_all_pointers()
{
	using namespace test_remove_all_pointers_globals;

	using T = double;
	static_assert(test<T, T*>);
	static_assert(test<T, T**>);
	static_assert(test<T, T**************>);
	static_assert(test<T, T* const*>);
	static_assert(test<T, T* const* volatile*>);
	static_assert(test<T, T* const* volatile* const volatile*>);
	static_assert(test<T[], T(* const* volatile* const volatile*)[]>);
	static_assert(test<const T, const T* const* volatile* const volatile*>);
	static_assert(!test<T, const T* const* volatile* const volatile*>);
	static_assert(!test<const T, T* const* volatile* const volatile*>);

	return true;
}

template <typename T>
consteval bool for_all_cvptr_permutations_of()
{
	using fgl::traits::pointer_to_byte;
	static_assert(pointer_to_byte<T*>);
	static_assert(pointer_to_byte<const T*>);
	static_assert(pointer_to_byte<volatile T*>);
	static_assert(pointer_to_byte<const volatile T*>);
	static_assert(pointer_to_byte<T* const>);
	static_assert(pointer_to_byte<T* volatile>);
	static_assert(pointer_to_byte<T* const volatile>);
	static_assert(pointer_to_byte<const T* const>);
	static_assert(pointer_to_byte<volatile T* volatile>);
	static_assert(pointer_to_byte<const volatile T* const volatile>);
	return true;
}

consteval bool test_pointer_to_byte()
{
	static_assert(for_all_cvptr_permutations_of<std::byte>());
	static_assert(for_all_cvptr_permutations_of<char>());
	static_assert(for_all_cvptr_permutations_of<unsigned char>());
	static_assert(for_all_cvptr_permutations_of<void>());

	using fgl::traits::pointer_to_byte;
	static_assert(!pointer_to_byte<signed char>);
	static_assert(!pointer_to_byte<int*>);
	static_assert(!pointer_to_byte<char**>);
	static_assert(!pointer_to_byte<double*>);

	return true;
}

consteval bool test_pointer_to_non_void()
{
	using fgl::traits::pointer_to_non_void;
	static_assert(!pointer_to_non_void<void*>);
	static_assert(!pointer_to_non_void<int>);
	static_assert(pointer_to_non_void<int*>);
	static_assert(pointer_to_non_void<const int*>);
	static_assert(pointer_to_non_void<const volatile int*>);
	static_assert(pointer_to_non_void<const volatile int* const>);
	static_assert(pointer_to_non_void<const volatile int* const volatile>);
	return true;
}

consteval bool test_pointer_traits()
{
	static_assert(test_remove_cvptr_t());
	static_assert(test_remove_all_pointers());
	static_assert(test_pointer_to_byte());
	static_assert(test_pointer_to_non_void());
	return true;
}

int main()
{
	static_assert(test_type_concepts());
	static_assert(test_ref_traits());
	static_assert(test_pointer_traits());
	static_assert(test_null_comparable());
	return EXIT_SUCCESS;
}

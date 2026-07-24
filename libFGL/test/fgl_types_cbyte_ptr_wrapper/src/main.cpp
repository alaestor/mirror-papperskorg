#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE, byte
#include <concepts> // same_as

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/types/cbyte_ptr_wrapper.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

using fgl::cbyte_ptr_wrapper;

template <typename T>
constexpr bool test_conversions()
{
	cbyte_ptr_wrapper<T*> wrapper{ nullptr };
	[[maybe_unused]] void* p1{ wrapper };
	[[maybe_unused]] const void* p1c{ wrapper };
	[[maybe_unused]] const volatile void* p1cv{ wrapper };
	if (std::same_as<T*, char*> || !std::is_constant_evaluated())
	{
		[[maybe_unused]] char* p2{ wrapper };
		[[maybe_unused]] const char* p2c{ wrapper };
	}
	else if (std::same_as<T*, unsigned char*> || !std::is_constant_evaluated())
	{
		[[maybe_unused]] unsigned char* p2{ wrapper };
		[[maybe_unused]] const unsigned char* p2c{ wrapper };
	}
	else if (std::same_as<T*, std::byte*> || !std::is_constant_evaluated())
	{
		[[maybe_unused]] std::byte* p2{ wrapper };
		[[maybe_unused]] const std::byte* p2c{ wrapper };
	}
	return true;
}

template <typename T>
constexpr bool test_comparisons()
{
	cbyte_ptr_wrapper<T*> wrapper(nullptr);

	constexpr_assert(wrapper.underlying_pointer == nullptr);
	constexpr_assert(wrapper == nullptr);
	constexpr_assert(nullptr == wrapper);

	void* vp{ nullptr };
	char* cp{ nullptr };
	unsigned char* ucp{ nullptr };
	std::byte* bp{ nullptr };

	constexpr_assert(wrapper == vp);
	constexpr_assert(wrapper == cp);
	constexpr_assert(wrapper == ucp);
	constexpr_assert(wrapper == bp);
	return true;
}

template <typename T>
constexpr bool test_arithmetic()
{
	std::ptrdiff_t offset{ 1 };
	T b{};
	T* expected{ &b };
	cbyte_ptr_wrapper wrapper(expected);

	constexpr_assert(
		static_cast<void*>(&*wrapper)
		== static_cast<void*>(&*expected)
	);

	constexpr_assert(
		static_cast<void*>(&wrapper[0])
		== static_cast<void*>(&expected[0])
	);

	constexpr_assert(
		static_cast<void*>(wrapper + offset)
		== static_cast<void*>(expected + offset)
	);

	constexpr_assert(
		static_cast<void*>(wrapper - offset)
		== static_cast<void*>(expected - offset)
	);

	constexpr_assert(
		static_cast<void*>(wrapper += offset)
		== static_cast<void*>(expected += offset)
	);

	constexpr_assert(
		static_cast<void*>(wrapper -= offset)
		== static_cast<void*>(expected -= offset)
	);

	constexpr_assert(
		static_cast<void*>(offset + wrapper)
		== static_cast<void*>(offset + expected)
	);

	constexpr_assert(
		wrapper + offset - wrapper
		== expected + offset - expected
	);

	constexpr_assert(
		static_cast<void*>(wrapper++)
		== static_cast<void*>(expected++)
	);

	constexpr_assert(
		static_cast<void*>(++wrapper)
		== static_cast<void*>(++expected)
	);

	constexpr_assert(
		static_cast<void*>(wrapper--)
		== static_cast<void*>(expected--)
	);

	constexpr_assert(
		static_cast<void*>(--wrapper)
		== static_cast<void*>(--expected)
	);

	expected = &b;
	wrapper = &b;

	constexpr_assert(
		static_cast<void*>(wrapper)
		== static_cast<void*>(expected)
	);

	wrapper = nullptr;

	void* vp{ nullptr };
	char* cp{ nullptr };
	unsigned char* ucp{ nullptr };
	std::byte* bp{ nullptr };

	if (std::same_as<T, char> || !std::is_constant_evaluated())
	{
		[[maybe_unused]] auto rcp = wrapper - cp;
		(void)(wrapper = cp);
	}

	if (std::same_as<T, unsigned char> || !std::is_constant_evaluated())
	{
		[[maybe_unused]] auto rucp = wrapper - ucp;
		(void)(wrapper = ucp);
	}

	if (std::same_as<T, std::byte> || !std::is_constant_evaluated())
	{
		[[maybe_unused]] auto rbp = wrapper - bp;
		(void)(wrapper = bp);
	}

	if (!std::is_constant_evaluated())
		(void)(wrapper = vp);

	return true;
}

template <typename T>
bool test_cbyte_ptr_wrapper()
{
	static_assert(test_conversions<T>());
	assert(test_conversions<T>());

	static_assert(test_comparisons<T>());
	assert(test_comparisons<T>());

	if constexpr (not std::same_as<T, void>)
	{
		static_assert(test_arithmetic<T>());
		assert(test_arithmetic<T>());
	}

	return true;
}

int main()
{
	assert(test_cbyte_ptr_wrapper<void>());
	assert(test_cbyte_ptr_wrapper<char>());
	assert(test_cbyte_ptr_wrapper<unsigned char>());
	assert(test_cbyte_ptr_wrapper<std::byte>());

	return EXIT_SUCCESS;
}

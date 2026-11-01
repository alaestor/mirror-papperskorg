#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE, byte
#include <cstddef> // byte
#include <array>
#include <ranges>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>
#include <fgl/debug/exception_occurs.hpp>

#include <fgl/utility/memory_copy.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

constexpr std::array<int, 5> data{ -1, -5059, 39210, 0, 50 };

bool test_memory_copy_same()
{
	// Should use the memcpy version
	std::array<int, 5> copy{};
	fgl::memory_copy(&copy, &data);
	assert(copy == data);

	// Should use the bitcast version
	[]() consteval
	{
		constexpr int expected{ 1337 };
		unsigned int result{};
		fgl::memory_copy(&result, &expected);
		constexpr_assert(static_cast<int>(result) == expected);
	}();
	return true;
}

bool test_memory_copy_bytes()
{
	std::array<int, 5> copy{};
	static_assert(sizeof(copy) == sizeof(data));
	fgl::unsafe::memory_copy(&copy, &data[0]);
	assert(copy == data);
	return true;
}

bool test_memory_copy_range()
{
	std::array<int, 5> copy{};
	static_assert(sizeof(copy) == sizeof(data));
	std::ranges::subrange byte_alias{
		reinterpret_cast<const std::byte*>(&data[0]),
		reinterpret_cast<const std::byte*>(&data[0]) + sizeof(copy)
	};
	fgl::memory_copy(&copy, byte_alias);
	assert(copy == data);
	return true;
}

bool test_memory_copy_exception()
{
	std::array<int, 5> copy{};
	static_assert(sizeof(copy) == sizeof(data));
	std::ranges::subrange byte_alias{
		reinterpret_cast<const std::byte*>(&data[0]),
		reinterpret_cast<const std::byte*>(&data[0]) + sizeof(copy) - 1
	};
	assert(byte_alias.size() < sizeof(copy));
	assert(
		specific_exception_occurs(
			std::out_of_range,
			(fgl::memory_copy(&copy, byte_alias))
		)
	);
	return true;
}

int main()
{
	assert(test_memory_copy_same());
	assert(test_memory_copy_bytes());
	assert(test_memory_copy_range());
	assert(test_memory_copy_exception());

	return EXIT_SUCCESS;
}

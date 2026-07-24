/**
This file is an example for <fgl/debug/constexpr_assert.hpp>

--- Example output: none; the program compiles and exits successfully
*/

// define enables the short "constexpr_assert"
// could also use #define FGL_SHORT_MACROS
#define FGL_DEBUG_CONSTEXPR_ASSERT_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

// add two integers and assert that the result is 2
constexpr int add_and_assert_two(int a, int b)
{
	const int result{ a + b };
	constexpr_assert(result == 2);
	// without short macros, use:
	// FGL_DEBUG_CONSTEXPR_ASSERT(result == 2);
	return result;
}

int main()
{
	[]() consteval
	{
		add_and_assert_two(1, 1); // OK
		//add_and_assert_two(5, 5); // COMPILE-TIME ERROR
	}();

	add_and_assert_two(1, 1); // OK
	//add_and_assert_two(5, 5); // RUNTIME ERROR (via assert)
}

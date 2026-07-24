/**
This file is an example for <fgl/debug/output.hpp>

--- Example output
-------------------------------------------------------------------------------
[FIXME] file:src/main.cpp(30:2) 'int main()
[FIXME] file:src/main.cpp(31:2) 'int main()
[FIXME] file:src/main.cpp(32:22) 'int main()
[FIXME] file:src/main.cpp(25:2) 'int add(int, int)
 \_____ return a - b;
1 + 2 = -1
[FIXME] file:src/main.cpp(35:2) 'int main()
 \_____ the author is an idiot
*/

#include <iostream>

// define enables the short "FIX", "FIX_ME", and "FIX_THIS"
// could also #define FGL_SHORT_MACROS
#define FGL_DEBUG_FIXME_SHORT_MACROS
#include <fgl/debug/fixme.hpp>

int add(int a, int b)
{
	FIX_THIS( return a - b; )
}

int main()
{
	FIX_ME;
	FIX_ME const int one{ 1};
	const int two{ 2 }; FIX_ME
	const int three{ add(one, two) };
	std::cout << one << " + " << two << " = " << three << std::endl;
	FIX("the author is an idiot")
}

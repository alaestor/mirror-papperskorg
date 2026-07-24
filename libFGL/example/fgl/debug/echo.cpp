/**
This file is an example for <fgl/debug/output.hpp>

--- Example output
-------------------------------------------------------------------------------
[ECHO] file:src/main.cpp(34:2) 'int main()
 \_____ a message
[ECHO] file:src/main.cpp(35:2) 'int main()
 \_____ another message
[ECHO] file:src/main.cpp(28:2) 'int add(int, int)
 \_____ result == 3
[ECHO] file:src/main.cpp(37:2) 'int main()
 \_____ x = add(1, 2) == 3
[ECHO] file:src/main.cpp(28:2) 'int add(int, int)
 \_____ result == 6
[ECHO] file:src/main.cpp(38:2) 'int main()
 \_____ add(x,x) == 6
*/

// define enables the short "ECHO" and "ECHOV"
// could also #define FGL_SHORT_MACROS
#define FGL_DEBUG_ECHO_SHORT_MACROS
#include <fgl/debug/echo.hpp>

int add(const int a, const int b)
{
	const auto result = a + b;
	ECHOV(result);
	return result;
}

int main()
{
	ECHO("a message");
	ECHO("another message");
	int x;
	ECHOV(x = add(1, 2));
	ECHOV(add(x,x));
}

/**
This file is an example for <fgl/types/string_literal.hpp>

--- Example output
-------------------------------------------------------------------------------
Hello, World!
*/
#include <iostream>

#include <fgl/types/string_literal.hpp>

template <fgl::string_literal message>
void print()
{
	std::cout << message << std::endl;
}

int main()
{
	print<"Hello, World!">();
}

/**
This file is an example for <fgl/environment/build_info.hpp>

--- Example output:
release build :)
misleading! :( Be careful of stupid code!
*/

#define NDEBUG // such definitions must be above the includes!
#include <iostream>

#include <fgl/environment/build_info.hpp>

int main()
{
	if constexpr (fgl::debug_build)
	{
		std::cout << "debug build, this wont print" << std::endl;
	}
	else if constexpr (fgl::release_build)
	{
		std::cout << "release build :)" << std::endl;
	}

// dont do this...
#undef NDEBUG
	if constexpr (fgl::release_build)
	{
		std::cout << "misleading! :( Be careful of stupid code!" << std::endl;
	}
}


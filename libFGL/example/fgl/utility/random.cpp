/**
This file is an example for <fgl/utility/random.hpp>

--- Example output
-------------------------------------------------------------------------------
random float between [0, 1]: 0.602908 0.726407 0.761528
random char between [a, z]: r m b
random ushort between [0, 65535]: 46408 43530 2389
deterministic: 0 1 2 3 4 5 6 7 8 9
*/

#include <iostream>
#include <string_view>

#include <fgl/utility/random.hpp>

void f(const std::string_view msg, auto&& rng)
{
	std::cout
		<< msg
		<< " between ["
		<< rng.distribution.min()
		<< ", "
		<< rng.distribution.max()
		<< "]: ";

	for (unsigned char i{ 0 }; i < 3; ++i)
		std::cout << rng() << ' '; // generate the random number and print it

	std::cout << '\n';
}

int main()
{
	f("random float", fgl::random<float>{ 0.0f, 1.0f});
	f("random char", fgl::random<char>{ 'a', 'z' }); // ASCII values
	f("random ushort", fgl::random<unsigned short>{});

	fgl::random<int> rng;

	/// Here, we'll use a 'debug object' to make the output deterministic
	const auto debug_object{
		[](int) noexcept
		{
			// the int parameter is unused here; it's the RNG number
			static int i{ 0 };
			return i++;
		}
	};

	std::cout << "deterministic: ";
	for (unsigned char i{ 0 }; i < 10; ++i)
		std::cout << rng(debug_object) << ' ';

	std::cout << std::endl;
}

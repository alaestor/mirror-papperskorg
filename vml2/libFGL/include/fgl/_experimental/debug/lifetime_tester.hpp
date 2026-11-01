#pragma once
#ifndef FGL_DEBUG_LIFETIME_TESTER_HPP_INCLUDED
#define FGL_DEBUG_LIFETIME_TESTER_HPP_INCLUDED

#include <iostream> //@TODO this should use the debug output stream

namespace fgl::debug{

/**
@file

@defgroup group-debug-lifetime_tester Lifetime Tester

@brief An object which records its constructions and destructions.

@{
*/

/**
@brief derp
*/
struct lifetime_tester
{
	static inline unsigned long long counter{ 0 };
	unsigned long long id{ counter++ };

	lifetime_tester()
	{ std::cout << "ctor()   ID: " << id << '\n'; }

	lifetime_tester(const lifetime_tester& other)
	{
		std::cout
			<< "ctor(&)  ID: " << id
			<< " (from ID: " << other.id << ")\n";
	}

	lifetime_tester(lifetime_tester&& other)
	{
		std::cout
			<< "ctor(&&) ID: " << id
			<< " (from ID: " << other.id << ")\n";
	}

	~lifetime_tester()
	{ std::cout << "~dtor()  ID: " << id << '\n'; }

	auto operator=(lifetime_tester) = delete;
};

///@}
} // namespace fgl::debug

#endif // FGL_DEBUG_LIFETIME_TESTER_HPP_INCLUDED

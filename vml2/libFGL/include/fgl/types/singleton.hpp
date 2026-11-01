#pragma once
#ifndef FGL_TYPES_SINGLETON_HPP_INCLUDED
#define FGL_TYPES_SINGLETON_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

namespace fgl {

/**
@file

@example example/fgl/types/singleton.cpp
	An example for @ref group-types-singleton

@defgroup group-types-singleton Singleton

@brief Type-based static singleton template

@details @parblock

	@attention Use of <tt>fgl::singleton</tt> is discouraged. A more idiomatic
		C++ design would be to use <tt>static</tt> members and methods with a
		deleted constructor. This template is provided because the idiomatic
		design may not always be possible due to other design constraints or
		situations where the more traditional "singleton pattern" may be more
		desirable for stylistic or clarity reasons.

	The <tt>fgl::singleton</tt> template can be used to create a
	"singleton instance" of a type. It doesn't require the template argument
	to be a typical "singleton-exclusive" object, or have private constructors,
	etc. Any type that satisfies the <tt>std::default_initializable</tt>
	concept can be used with the singleton template.

	For example, <tt>fgl::singleton<int></tt> is a valid singleton whose
	<tt>@ref instance()</tt> method will return an <tt>int&</tt> to a
	statically declared <tt>int</tt>.

	Contrary to the name, there can be multiple singleton instances of
	the same type, so desired.

	@note Despite the requirement that @p T satisfy the
		<tt>std::default_initializable</tt> concept, the template isn't
		constrained by it to avoid "incomplete type" issues which would
		otherwise complicate object definitions.

	@endparblock

@see the example program @ref example/fgl/types/singleton.cpp
@{
*/

#ifndef FGL_SINGLETON_BOILERPLATE
	/**
	@details
		Defines a default constructor, deletes the copy and move constructors,
		deletes the assignment operator, and friends the singleton template.
		Intended to be used within the <tt>private</tt> section when defining
		a singleton-exclusive type.
	@param class_name The name of the class
	*/
	#define FGL_SINGLETON_BOILERPLATE(class_name) \
		friend class fgl::singleton<class_name>;\
		class_name() = default;\
		class_name(const class_name&) = delete;\
		class_name(const class_name&&) = delete;\
		void operator=(const class_name&) = delete
#else
	#error FGL_SINGLETON_BOILERPLATE already defined
#endif // FGL_SINGLETON_BOILERPLATE

/**
@brief A singleton template which can access a static instance of @p T
@copydetails group-types-singleton
@tparam T The type to be statically instantiated
*/
template <typename T>
class singleton final
{
	singleton(auto&&...) = delete; ///< should never be instantiated

public:

	/// @returns A reference to the static singleton instance
	static T& instance() noexcept
	{
		static T instance{};
		return instance;
	}
};

/**
@brief A not-so-singleton template which can access multiple instances of @p T
@see fgl::singleton
@tparam T The type to be statically instantiated
*/
template <typename T>
class multiton final
{
	multiton(auto&&...) = delete; ///< should never be instantiated

public:

	/**
	@tparam T_guid A number to uniquely identify the instance
	@returns A reference to the @p T_guid static instance
	*/
	template <unsigned long long T_guid>
	static T& instance() noexcept
	{
		static T instance{};
		return instance;
	}
};

///@}

}// namespace fgl

#endif // FGL_TYPES_SINGLETON_HPP_INCLUDED

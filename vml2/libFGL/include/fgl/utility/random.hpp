#pragma once
#ifndef FGL_UTILITY_RANDOM_HPP_INCLUDED
#define FGL_UTILITY_RANDOM_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <cstdint> // uint_fast64_t
#include <concepts> // integral, floating_point
#include <limits> // numeric_limits
#include <type_traits> // conditional
#include <random>

#include "../types/traits.hpp" // numeric_type

namespace fgl {

/**
@file

@example example/fgl/utility/random.cpp
	An example for @ref group-utility-random

@defgroup group-utility-random Random

@brief A simplifying abstraction for standard pseudo-random numbers.

@see the example program @ref example/fgl/utility/random.cpp
@{
*/

///@internal @attention This header uses sadface initializer syntax (Line 69)

/**
@copybrief group-utility-random
@details A callable object that can generate pseudo-random numbers of type
	@p T from @p T_distribution using @p T_generator  between
	<tt>[min, max]</tt>
@tparam T The type of random value to generate
@tparam T_distribution The distribution to use
@tparam T_generator The generator to use
@tparam T_table_size The table size for the <tt>std::shuffle_order_engine</tt>
*/
template
<
	fgl::traits::numeric_type T = uint_fast64_t,
	class T_distribution =
		std::conditional
		<
			std::integral<T>,
			std::uniform_int_distribution<T>,
			std::uniform_real_distribution<T>
		>::type,
	class T_generator = std::mt19937_64,
	const std::size_t T_table_size = 8
>
struct random
{
	T_distribution distribution;
	std::shuffle_order_engine<T_generator, T_table_size> engine;

	/**
	@{ @name Constructors
	@param min Minimum value that can be generated, which defaults to the
		minimum numeric limit of @p T
	@param max Maximum value that can be generated,  which defaults to the
		maximum numeric limit of @p T
	@param seed Seed for the pseudo-random number generator, which defaults
		to @c std::random_device{}()
	*/
	[[nodiscard]] explicit random(
		const T min = std::numeric_limits<T>::min(),
		const T max = std::numeric_limits<T>::max(),
		const std::random_device::result_type seed = (std::random_device{}())
	):
		distribution(min, max),
		engine(seed)
	{}
	///@} Constructors

	/**
	@brief Generates a random value between <tt>[min, max]</tt>
	@returns The result of @c distribution(engine)
	*/
	[[nodiscard]] T operator()() noexcept(noexcept(distribution(engine)))
	{ return distribution(engine); }

	/**
	@details Calls the @p debug_object with the generated value, and returns
		whatever the debug object returns.
	@remarks Because the input and output may differ, this method can be used
		to override the random number generator to return a deterministic
		value. It could be used to passively collect statistical data about
		the random number generator.
	@tparam T_debug_object A callable object type that takes a single argument
		of type @p T and returns a value of type @p T.
	@param debug_object The callable object which takes the generated @p T
		value as an argument and returns a @p T
	@returns The result of @c debug_object(distribution(engine))
	*/
	template <class T_debug_object>
	requires requires(T t, T_debug_object obj){{ obj(t) } -> std::same_as<T>;}
	[[nodiscard]] T operator()(const T_debug_object& debug_object)
	noexcept(noexcept(debug_object(distribution(engine))))
	{ return debug_object(distribution(engine)); }
};

///@}
} // namespace fgl

#endif // FGL_UTILITY_RANDOM_HPP_INCLUDED

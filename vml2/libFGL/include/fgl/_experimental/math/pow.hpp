#pragma once
#ifndef FGL_MATH_POW_HPP_INCLUDED
#define FGL_MATH_POW_HPP_INCLUDED

#include <cstdint>
#include <concepts> // integral, floating_point
#include <type_traits> // conditional_t
#include <bit> // bitcast
#include <array>
#include <limits>

//#include <fgl/types/ctypes.hpp>
#include "../../types/traits.hpp" // numeric_type

namespace fgl {

// pow of an integer and unsigned integer
template <std::integral T, std::unsigned_integral T_exponent = T>
[[nodiscard]] constexpr inline T pow(T base, T_exponent exponent)
{
	if (exponent == 1) return base;

	T result{ 1 };
	while (exponent != 0)
	{
		if (exponent & 1)
			result *= base;

		exponent >>= 1;
		base *= base;
	}

	return result;
}

// pow of a numeric base and an integer exponent (works with negatives)
template <fgl::traits::numeric_type T, std::integral T_exponent = T>
[[nodiscard]] constexpr inline
std::conditional_t<std::floating_point<T>, double, T>
pow(const T base, const T_exponent exponent)
{
	if (exponent == 0) return 1;
	else if (exponent == 1) return base;

	T result{ 1 };
	if (exponent < 0)
	{
		for (T_exponent i{ exponent }; i < 0; ++i)
			result /= base;
	}
	else
	{
		for (T_exponent i{ 0 }; i < exponent; ++i)
			result *= base;
	}
	return result;
}

// pow approximation for little-endian IEEE-754 binary64
template <bool T_improved_precision = true>
[[nodiscard]] constexpr inline
double pow_approximation(double base, const double exponent)
{
	static_assert(
		std::numeric_limits<float>::is_iec559
		&& std::numeric_limits<double>::is_iec559
		&& std::numeric_limits<long double>::is_iec559,
		"fgl::pow_approximation() from " __FILE__
		" requires IEC-559 aka IEEE-754 floating point representations."
		" The target platform and/or vendor implementation is incompatible."
	);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
	if (exponent == 0) return 1;
	else if (exponent == 1) return base;
#pragma GCC diagnostic pop

	using half_t = uint_least32_t;
	static_assert(sizeof(half_t) == sizeof(decltype(base)) / 2);
	using array_t = std::array<half_t, 2>;

	auto intrep{ std::bit_cast<array_t>(base) };
	intrep[0] = 0;

	constexpr uint_fast32_t magic{ 1072632447 };

	// TODO could be generic for any floating point type; need to
	// find magic cconstants for float and long double, and selectively pick
	// half_t conditionally (16 for float, 32 for double, 64 for long double)

	if constexpr (T_improved_precision)
	{
		// is this still the case? should test
		// stolen comments from the code i adapted:
		//	  exponentiation by squaring with the exponent's integer part
		//	  double r = u.d makes everything much slower, not sure why
		uint_fast32_t exp_i{ static_cast<uint_fast32_t>(exponent) };
		intrep[1] = static_cast<half_t>(
			(exponent - exp_i) * (intrep[1] - magic) + magic
		);
		double r{ 1.0 };
		while (exp_i)
		{
			if (exp_i & 1)
			{
				r *= base;
			}
			base *= base;
			exp_i >>= 1;
		}
		return r * std::bit_cast<double>(intrep);
	}
	else
	{
		intrep[1] = static_cast<half_t>(exponent * (intrep[1] - magic) + magic);
		return std::bit_cast<double>(intrep);
	}
}

} // namespace fgl

// std::cout
// 	<< "\nNormal:"
// 	<< "\n\t8.0  ^ 3	= " << std::pow(8.0, 3)
// 	<< "\n\t3.05 ^ 1.98 = " << std::pow(3.05, 1.98);

// std::cout
// 	<< "\nFast, inaccurate:"
// 	<< "\n\t8.0  ^ 3	= " << pow_approximation<false>(8.0, 3.0)
// 	<< "\n\t3.05 ^ 1.98 = " << pow_approximation<false>(3.05, 1.98);

// std::cout
// 	<< "\nFast, more accurate:"
// 	<< "\n\t8.0  ^ 3	= " << pow_approximation<true>(8.0, 3.0)
// 	<< "\n\t3.05 ^ 1.98 = " << pow_approximation<true>(3.05, 1.98);

// std::cout << '\n' << std::endl;

#endif // FGL_MATH_POW_HPP_INCLUDED

#pragma once
#ifndef FGL_MATH_SQRT_HPP_INCLUDED
#define FGL_MATH_SQRT_HPP_INCLUDED

#include "../../types/traits.hpp" // numeric_type
#include "./abs.hpp"

namespace fgl {

///@returns The square root of @p x .
template <fgl::traits::numeric_type T>
[[nodiscard]] constexpr inline T sqrt(const T x)
{
	if (x == 0)
		return x;

	const double dividend{ static_cast<double>(x) };
	double value{ static_cast<double>(x) };
	double last;
	do
	{
		last = value;
		value = (value + dividend / value) * 0.5;
	} while(fgl::abs(value - last) > 1e-9);
	return static_cast<T>(value);
}

} // namespace fgl

#endif // FGL_MATH_SQRT_HPP_INCLUDED

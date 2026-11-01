#pragma once
#ifndef FGL_MATH_ABS_HPP_INCLUDED
#define FGL_MATH_ABS_HPP_INCLUDED

#include "../../types/traits.hpp" // numeric_type

namespace fgl {

///@returns The absolute value of @p x .
template <fgl::traits::numeric_type T>
[[nodiscard]] constexpr inline T abs(T x)
{ return x < 0 ? -x : x; }

} // namespace fgl

#endif // FGL_MATH_ABS_HPP_INCLUDED

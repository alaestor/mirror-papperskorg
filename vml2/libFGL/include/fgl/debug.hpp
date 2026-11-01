#pragma once
#ifndef FGL_DEBUG_HPP_INCLUDED
#define FGL_DEBUG_HPP_INCLUDED

// should contain all `debug/*.hpp`

/**
@page page-fgl-header-debug libFGL Debug fascilities
@details
	<tt>#include <fgl/debug.hpp></tt> provides the following:
	- @ref group-debug-constexpr_assert
	- @ref group-debug-echo
	- @ref group-debug-exception_occurs
	- @ref group-debug-fixme
	- @ref group-debug-output
	- @ref group-debug-stopwatch
*/

#include "./debug/constexpr_assert.hpp"
#include "./debug/echo.hpp"
#include "./debug/exception_occurs.hpp"
#include "./debug/fixme.hpp"
#include "./debug/output.hpp"
#include "./debug/stopwatch.hpp"

#endif // FGL_DEBUG_HPP_INCLUDED

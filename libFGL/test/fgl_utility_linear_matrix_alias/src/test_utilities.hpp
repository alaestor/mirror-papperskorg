#pragma once
#ifndef TESTS_FGL_UTILITY_LINEAR_MATRIX_ALIAS_UTILITIES_HPP_INCLUDED
#define TESTS_FGL_UTILITY_LINEAR_MATRIX_ALIAS_UTILITIES_HPP_INCLUDED

#include <fgl/utility/linear_matrix_alias/utilities.hpp>

namespace test_matrix_utilities {

enum e_dimensions : bool
{
	static_dimensions = true,
	dynamic_dimensions = false
};

template
<
	fgl::major T_major,
	std::unsigned_integral T_bound_type,
	T_bound_type ... T_bounds
>
consteval bool test_offsets_impl(
	const std::array<T_bound_type, sizeof...(T_bounds)>& expected)
{
	constexpr std::size_t dimensions{ sizeof...(T_bounds) };
	constexpr std::array<T_bound_type, dimensions> bounds{ T_bounds... };
	using fgl::linear_matrix_utilities::make_offsets;
	constexpr auto result{ make_offsets<dimensions, T_major>(bounds) };
	return result == expected;
}

template <std::unsigned_integral T>
consteval bool test_offsets()
{
	using enum fgl::major;
	// 3D
	static_assert(test_offsets_impl<column, T, 3, 4, 5>({1, 3, 12}));
	static_assert(test_offsets_impl<row, T, 3, 4, 5>({20, 5, 1}));
	static_assert(test_offsets_impl<column, T, 4, 4, 4>({1, 4, 16}));
	static_assert(test_offsets_impl<row, T, 4, 4, 4>({16, 4, 1}));
	static_assert(test_offsets_impl<column, T, 5, 5, 5>({1, 5, 25}));
	static_assert(test_offsets_impl<row, T, 5, 5, 5>({25, 5, 1}));
	static_assert(test_offsets_impl<column, T, 5, 4, 3>({1, 5, 20}));
	static_assert(test_offsets_impl<row, T, 5, 4, 3>({12, 3, 1}));
	// 4D
	static_assert(test_offsets_impl<column, T, 2, 3, 4, 5>({1, 2, 6, 24}));
	static_assert(test_offsets_impl<row, T, 2, 3, 4, 5>({60, 20, 5, 1}));
	static_assert(test_offsets_impl<column, T, 4, 4, 4, 4>({1, 4, 16, 64}));
	static_assert(test_offsets_impl<row, T, 4, 4, 4, 4>({64, 16, 4, 1}));
	static_assert(test_offsets_impl<column, T, 5, 5, 5, 5>({1, 5, 25, 125}));
	static_assert(test_offsets_impl<row, T, 5, 5, 5, 5>({125, 25, 5, 1}));
	static_assert(test_offsets_impl<column, T, 5, 4, 3, 2>({1, 5, 20, 60}));
	static_assert(test_offsets_impl<row, T, 5, 4, 3, 2>({24, 6, 2, 1}));
	// 5D
	static_assert(test_offsets_impl<column, T, 2, 3, 4, 5, 6>({1, 2, 6, 24, 120}));
	static_assert(test_offsets_impl<row, T, 2, 3, 4, 5, 6>({360, 120, 30, 6, 1}));
	static_assert(test_offsets_impl<column, T, 4, 4, 4, 4, 4>({1, 4, 16, 64, 256}));
	static_assert(test_offsets_impl<row, T, 4, 4, 4, 4, 4>({256, 64, 16, 4, 1}));
	static_assert(test_offsets_impl<column, T, 5, 5, 5, 5, 5>({1, 5, 25, 125, 625}));
	static_assert(test_offsets_impl<row, T, 5, 5, 5, 5, 5>({625, 125, 25, 5, 1}));
	static_assert(test_offsets_impl<column, T, 6, 5, 4, 3, 2>({1, 6, 30, 120, 360}));
	static_assert(test_offsets_impl<row, T, 6, 5, 4, 3, 2>({120, 24, 6, 2, 1}));
	return true;
}

template <std::size_t T_dimensions, fgl::major T_major = fgl::major::row>
constexpr bool test_dim_impl(
	const std::size_t expected,
	const std::array<std::size_t, T_dimensions>& indexes,
	const std::array<std::size_t, T_dimensions>& offsets)
{
	using namespace fgl::linear_matrix_utilities;
	return (expected == matrix_to_linear(indexes, offsets));
}


constexpr bool test_dimensional_to_linear()
{
	using fgl::linear_matrix_utilities::make_offsets;
	constexpr std::array<std::size_t, 6> bounds{ 2,3,4,5,6,7 };
	{
	constexpr auto offsets{ make_offsets<6, fgl::major::row>(bounds) };
	constexpr_assert(test_dim_impl<6>(0, { 0,0,0,0,0,0 }, offsets));
	constexpr_assert(test_dim_impl<6>(3620, { 1,1,1,1,1,1 }, offsets));
	constexpr_assert(test_dim_impl<6>(3677, { 1,1,1,2,3,2 }, offsets));
	constexpr_assert(test_dim_impl<6>(5039, { 1,2,3,4,5,6 }, offsets));
	}
	{
	constexpr auto offsets{ make_offsets<6, fgl::major::column>(bounds) };
	constexpr_assert(test_dim_impl<6>(0, { 0,0,0,0,0,0 }, offsets));
	constexpr_assert(test_dim_impl<6>(873, { 1,1,1,1,1,1 }, offsets));
	constexpr_assert(test_dim_impl<6>(884, { 2,3,2,1,1,1 }, offsets));
	constexpr_assert(test_dim_impl<6>(1072, { 6,5,4,3,2,1 }, offsets));
	}
	return true;
}

constexpr bool test_passes()
{
	static_assert(test_offsets<unsigned short>());
	static_assert(test_offsets<unsigned long long>());
	static_assert(test_dimensional_to_linear());
	return true;
}

} // namespace test_matrix_utilities

#endif // TESTS_FGL_UTILITY_LINEAR_MATRIX_ALIAS_UTILITIES_HPP_INCLUDED

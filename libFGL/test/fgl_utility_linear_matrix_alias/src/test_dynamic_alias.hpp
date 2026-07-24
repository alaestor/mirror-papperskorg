#pragma once
#ifndef TESTS_FGL_UTILITY_LINEAR_MATRIX_DYNAMIC_HPP_INCLUDED
#define TESTS_FGL_UTILITY_LINEAR_MATRIX_DYNAMIC_HPP_INCLUDED

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/utility/linear_matrix_alias/dynamic.hpp>
#include "./utility.hpp"

#include <concepts> // same_as

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

namespace test_dynamic_alias {

template
<
	typename T,
	std::size_t T_dimensions,
	std::size_t ... T_bounds
>
constexpr void test_row_major_index_behavior(
	auto& buffer, auto& alias)
{
	T* last_address{ buffer.data() - 1 };
	iterate_bounds<T_bounds...>(
		[&](const std::array<std::size_t, T_dimensions>& indicies)
		{
			T* const current_address{ &alias.at(indicies) };
			constexpr_assert(current_address == &alias[indicies]);
			constexpr_assert(current_address == last_address + 1);
			last_address = current_address;
		}
	);
	constexpr_assert(last_address == &buffer.back()) //row major index behavior
}

/// REMINDER: defines arent scoped
#define D_BOUNDS1 5,3,4,2
#define D_BOUNDS1_PRODUCT 5*3*4*2
#define D_BOUNDS2 5,2,3,2
#define D_BOUNDS2_PRODUCT 5*2*3*2

constexpr bool general_test_impl()
{
	std::vector<unsigned short> buffer{};
	constexpr std::size_t n{ 4 };
	buffer.resize(D_BOUNDS1_PRODUCT);
	using enum fgl::major;
	auto alias = fgl::make_matrix_alias(buffer, row, D_BOUNDS1);
	static_assert(
		std::same_as<
			decltype(alias),
			fgl::dynamic_linear_matrix_alias<decltype(buffer)::iterator, n>
		>
	);
	constexpr_assert(alias.iterator() == buffer.begin());
	constexpr_assert(alias.is_row_major() && !alias.is_column_major());
	test_row_major_index_behavior<unsigned short, n, D_BOUNDS1>(buffer, alias);

	// test resize
	buffer.resize(D_BOUNDS2_PRODUCT);
	alias.set_bounds({D_BOUNDS2});
	constexpr_assert(alias.iterator() == buffer.begin());
	constexpr_assert(alias.is_row_major() && !alias.is_column_major());
	test_row_major_index_behavior<unsigned short, n, D_BOUNDS2>(buffer, alias);

	// test switch to column major
	alias.switch_major();
	constexpr_assert((!alias.is_row_major()) && alias.is_column_major());
	{  // <index behavior>
		unsigned short counter{ 0 };
		iterate_bounds<D_BOUNDS2>(
			[&](const std::array<std::size_t, 4>& indicies)
			{
				auto& r{ alias.at(indicies) };
				constexpr_assert(&r == &alias[indicies]);
				r = counter++;
			}
		);
		std::vector<unsigned short> expected{ 0,12,24,36,48,6,18,30,42,54,2,14,26,38,50,8,20,32,44,56,4,16,28,40,52,10,22,34,46,58,1,13,25,37,49,7,19,31,43,55,3,15,27,39,51,9,21,33,45,57,5,17,29,41,53,11,23,35,47,59 };
		constexpr_assert(buffer == expected); // column major index behavior
	} // </index behavior>

	// interface check
	{
		constexpr_assert(alias.iterator() == buffer.begin());
		using array_t = decltype(alias)::array_t;
		constexpr_assert((alias.bounds() == array_t{D_BOUNDS2}));
		constexpr_assert(alias.dimensions() == 4);

		std::vector<unsigned short> buffer1;
		buffer1.resize(3);
		constexpr array_t bounds1{1,3,1,1};
		alias.update(buffer1, bounds1);
		constexpr_assert(alias.iterator() == buffer1.begin());
		constexpr_assert(alias.bounds() == bounds1);

		std::vector<unsigned short> buffer2;
		buffer2.resize(3*5);
		constexpr array_t bounds2{3,1,5,1};
		alias.update(buffer2, bounds2, column);
		constexpr_assert(alias.iterator() == buffer2.begin());
		constexpr_assert(alias.bounds() == bounds2);
		constexpr array_t bounds2_a{1,3,1,5};
		alias.set_bounds(bounds2_a);
		constexpr_assert(alias.bounds() == bounds2_a);
	}

	return true;
}

template <fgl::major T_major, std::size_t ... T_valid_bounds>
[[nodiscard]] bool test_exceptions_impl(auto& invalid_sized_range) try
{
	// ctor exception
	got_expected_exception((
		(void)fgl::make_matrix_alias(
			invalid_sized_range,
			T_major,
			T_valid_bounds...
		)
	));

	// strong mutate exception guarentee (only for range interfaces)
	constexpr std::array<char, (T_valid_bounds * ...)> buffer{};
	auto alias{ fgl::make_matrix_alias(buffer, T_major, T_valid_bounds...) };
	using array_t = decltype(alias)::array_t;
	constexpr array_t original_bounds{ T_valid_bounds... };
	const array_t original_offsets{ alias.offsets() };
	const auto original_iterator{ alias.iterator() };

	got_expected_exception((alias.set_iterator(invalid_sized_range)));
	assert(alias.bounds() == original_bounds);
	assert(alias.offsets() == original_offsets);
	assert(alias.iterator() == original_iterator);

	got_expected_exception((alias.update(invalid_sized_range, original_bounds)));
	assert(alias.bounds() == original_bounds);
	assert(alias.offsets() == original_offsets);
	assert(alias.iterator() == original_iterator);

	got_expected_exception((
		alias.update(
			invalid_sized_range,
			original_bounds,
			fgl::major::row
		)
	));
	assert(alias.bounds() == original_bounds);
	assert(alias.offsets() == original_offsets);
	assert(alias.iterator() == original_iterator);

	return true;
}
catch( ... )
{ return false; }

template <std::size_t T_offset, std::size_t ... T_valid_bounds>
bool test_exceptions()
{
	using enum fgl::major;
	constexpr std::array<char, (T_valid_bounds * ...)-T_offset> too_small{};
	constexpr std::array<char, (T_valid_bounds * ...)+T_offset> too_big{};
	assert((test_exceptions_impl<row, T_valid_bounds...>(too_small)));
	assert((test_exceptions_impl<column, T_valid_bounds...>(too_small)));
	assert((test_exceptions_impl<row, T_valid_bounds...>(too_big)));
	assert((test_exceptions_impl<column, T_valid_bounds...>(too_big)));
	return true;
}

bool test_passes()
{
	static_assert(general_test_impl());
	assert((test_exceptions<1, 2,2>()));
	assert((test_exceptions<4, 2,2>()));
	assert((test_exceptions<1, 3,3,3>()));
	assert((test_exceptions<27, 3,3,3>()));
	return true;
}

} // namespace test_dynamic_alias
#endif // TESTS_FGL_UTILITY_LINEAR_MATRIX_DYNAMIC_HPP_INCLUDED

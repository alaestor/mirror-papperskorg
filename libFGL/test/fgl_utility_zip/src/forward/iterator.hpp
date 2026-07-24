#pragma once
#ifndef FGL_TEST_ZIP_FORWARD_ITERATOR_HPP_INCLUDED
#define FGL_TEST_ZIP_FORWARD_ITERATOR_HPP_INCLUDED

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE, size_t
#include <iterator> // forward_iterator
#include <algorithm> // min

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/utility/zip.hpp>

#include <fgl/types/traits.hpp>
using fgl::traits::is_const_ref, fgl::traits::is_nonconst_ref;

#include "../types.hpp"

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

namespace forward::iterator {

using std::forward_iterator, fgl::internal::forward_zip_iterator;

using namespace container_and_range_types;

template <typename ... T_iters>
inline constexpr bool can_be_forward_zip_iter_t{
	forward_iterator<forward_zip_iterator<T_iters...>>
};

consteval bool can_have_mixed_types()
{
	static_assert(can_be_forward_zip_iter_t<array_begin_it_t>);
	static_assert(can_be_forward_zip_iter_t<map_begin_it_t>);
	static_assert(can_be_forward_zip_iter_t<list_begin_it_t>);
	static_assert(can_be_forward_zip_iter_t<vector_begin_it_t>);
	static_assert(can_be_forward_zip_iter_t<deque_begin_it_t>);
	static_assert(
		can_be_forward_zip_iter_t
		<
			array_begin_it_t,
			map_begin_it_t,
			list_begin_it_t,
			vector_begin_it_t,
			deque_begin_it_t
		>
	);
	return true;
}

consteval bool can_have_mixed_constness()
{
	static_assert(can_be_forward_zip_iter_t<array_cbegin_it_t>);
	static_assert(can_be_forward_zip_iter_t<map_begin_it_t>);
	static_assert(can_be_forward_zip_iter_t<list_cbegin_it_t>);
	static_assert(can_be_forward_zip_iter_t<vector_begin_it_t>);
	static_assert(can_be_forward_zip_iter_t<deque_cbegin_it_t>);
	static_assert(
		can_be_forward_zip_iter_t
		<
			array_cbegin_it_t,
			map_begin_it_t,
			list_cbegin_it_t,
			vector_begin_it_t,
			deque_cbegin_it_t
		>
	);

	return true;
}

consteval bool expected_behavior()
{
	constexpr std::size_t short_len{ 5 };
	constexpr std::size_t medium_len{ 10 };
	constexpr std::size_t long_len{ 15 };
	std::array<double, short_len> a1{ 0 };
	std::array<char, medium_len> a2{ 0 };
	std::array<int, long_len> a3{ 0 };

	using expected_iter_value_t =
		std::tuple
		<
			decltype(*a1.begin()),
			decltype(*a2.cbegin()),
			decltype(*a3.begin())
		>;

	const forward_zip_iterator beginning(a1.begin(), a2.cbegin(), a3.begin());
	using zip_iter_t = typename std::remove_const_t<decltype(beginning)>;
	static_assert(forward_iterator<zip_iter_t>);
	static_assert(std::is_same_v<decltype(*beginning), expected_iter_value_t>);

	using zip_sentinel_t = typename decltype(beginning)::difference_type;
	static_assert(std::sentinel_for<zip_sentinel_t, zip_iter_t>);
	constexpr zip_sentinel_t shortest{ short_len };

	// test zip iterator equality comparisons
	{
		zip_iter_t it = beginning;
		zip_iter_t end(a1.end(), a2.cend(), a3.end());
		static_assert(std::sentinel_for<decltype(end), decltype(it)>);

		std::size_t i{ 0 };
		#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		for (; it != end; ++i, ++it);
		#pragma GCC diagnostic pop // ignored "-Wdeprecated-declarations"
		constexpr_assert(i == short_len);
	}

	// test zip iterator difference_type sentinel equality comparisons
	{
		zip_iter_t it = beginning;
		std::size_t i{ 0 };
		for (; it != shortest; ++i, ++it);
		constexpr_assert(i == short_len);
	}

	// test zip value_type correctness
	{
		zip_iter_t it = beginning;
		std::size_t i{ 0 };
		for (; it != shortest; ++i, ++it)
		{
			const auto& [a1_r, a2_r, a3_r]{ *it };
			static_assert(is_nonconst_ref<decltype(a1_r)>);
			static_assert(is_const_ref<decltype(a2_r)>);
			static_assert(is_nonconst_ref<decltype(a3_r)>);
			constexpr_assert(&a1_r == &a1[i]);
			constexpr_assert(&a2_r == &a2[i]);
			constexpr_assert(&a3_r == &a3[i]);
			constexpr_assert(it.index() == static_cast<zip_sentinel_t>(i));
		}
		constexpr_assert(i == short_len);
	}

	return true;
}

consteval bool test_passes()
{
	static_assert(can_have_mixed_types());
	static_assert(can_have_mixed_constness());
	static_assert(expected_behavior());
	return true;
}

} // namespace forward::iterator

#endif // FGL_TEST_ZIP_FORWARD_ITERATOR_HPP_INCLUDED

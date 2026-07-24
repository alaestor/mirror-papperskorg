#pragma once
#ifndef FGL_TEST_ZIP_FORWARD_RANGE_HPP_INCLUDED
#define FGL_TEST_ZIP_FORWARD_RANGE_HPP_INCLUDED

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE, size_t
#include <iterator> // forward_iterator
#include <ranges>
#include <algorithm>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/utility/zip.hpp>

#include <fgl/types/traits.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

using fgl::traits::is_const_ref, fgl::traits::is_nonconst_ref;

#include "../types.hpp"

namespace forward::range {

using std::ranges::subrange;

using
	fgl::zip,
	fgl::czip,
	fgl::internal::forward_zip_iterator;

using
	std::forward_iterator,
	std::ranges::forward_range;

using namespace container_and_range_types;

namespace globals {
array_t array{ 0,1,2,3,4,5,6,7 };
map_t map{ {0,'a'}, {1,'b'}, {3,'c'}, {4,'d'} };
list_t list{ 0,1,2,3,4,5,6,7,8,9 };
vector_t vector{ 'a','b','c','d','e','f' };
deque_t deque{ 0,1,2,3,4,5,6 };

const std::size_t smallest_length{
	std::min(
		{
			array.size(),
			map.size(),
			list.size(),
			vector.size(),
			deque.size()
		}
	)
};
}// namespace globals


bool zip_expected_behavior()
{
	using namespace globals;

	using zip_it_begin_t =
		forward_zip_iterator
		<
			array_begin_it_t,
			map_begin_it_t,
			list_begin_it_t,
			vector_begin_it_t,
			deque_begin_it_t
		>;
	static_assert(forward_iterator<zip_it_begin_t>);

	using zip_sentinel_t = typename zip_it_begin_t::difference_type;
	static_assert(std::sentinel_for<zip_sentinel_t, zip_it_begin_t>);

	using zip_expected_type = subrange<zip_it_begin_t, zip_sentinel_t>;
	static_assert(forward_range<zip_expected_type>);

	auto zipped{ zip(smallest_length, array, map, list, vector, deque) };
	static_assert(std::is_same_v<decltype(zipped), zip_expected_type>);

	static_assert(std::is_same_v<decltype(zipped.begin()), zip_it_begin_t>);
	const zip_it_begin_t begin_zip_it{
		array.begin(), map.begin(), list.begin(), vector.begin(), deque.begin()
	};
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	constexpr_assert(zipped.begin() == begin_zip_it);
	#pragma GCC diagnostic pop // ignored "-Wdeprecated-declarations"

	{
		auto auto_sized{ zip(array, map, list, vector, deque) };
		static_assert(std::is_same_v<decltype(zipped), decltype(auto_sized)>);
		#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		constexpr_assert(zipped.begin() == auto_sized.begin());
		#pragma GCC diagnostic pop // ignored "-Wdeprecated-declarations"
		constexpr_assert(zipped.end() == auto_sized.end());
	}

	static_assert(std::is_same_v<decltype(zipped.end()), zip_sentinel_t>);
	const zip_sentinel_t end_zip_it{
		static_cast<zip_sentinel_t>(std::min({
			array.size(), map.size(), list.size(), vector.size(), deque.size()
		}))
	};
	constexpr_assert(zipped.end() == end_zip_it);

	array_begin_it_t array_it{ array.begin() };
	map_begin_it_t map_it{ map.begin() };
	list_begin_it_t list_it{ list.begin() };
	vector_begin_it_t vector_it{ vector.begin() };
	deque_begin_it_t deque_it{ deque.begin() };

	std::size_t loop_count{ 0 };
	for (const auto& [array_r, map_r, list_r, vector_r, deque_r] : zipped)
	{
		static_assert(!std::is_const_v<array_t>);
		static_assert(is_nonconst_ref<decltype(array_r)>);
		constexpr_assert(&array_r == &*array_it++);

		static_assert(!std::is_const_v<map_t>);
		static_assert(is_nonconst_ref<decltype(map_r)>);
		const auto& [map_k, map_v]{ map_r };
		const auto& [map_it_k, map_it_v]{ *map_it++ };
		constexpr_assert(map_k == map_it_k);
		constexpr_assert(&map_v == &map_it_v);

		static_assert(std::is_const_v<list_t>);
		static_assert(is_const_ref<decltype(list_r)>);
		constexpr_assert(&list_r == &*list_it++);

		static_assert(std::is_const_v<vector_t>);
		static_assert(is_const_ref<decltype(vector_r)>);
		constexpr_assert(&vector_r == &*vector_it++);

		static_assert(!std::is_const_v<deque_t>);
		static_assert(is_nonconst_ref<decltype(deque_r)>);
		constexpr_assert(&deque_r == &*deque_it++);

		++loop_count;
	}
	constexpr_assert(loop_count == smallest_length);

	return true;
}

consteval bool zip_rvalues()
{

	using rval_begin_iter_t = decltype(std::begin(array_t{1,2,3,4,5,6,7}));
	using zip_it_begin_t = forward_zip_iterator<rval_begin_iter_t>;
	static_assert(forward_iterator<zip_it_begin_t>);

	using zip_sentinel_t = typename zip_it_begin_t::difference_type;
	static_assert(std::sentinel_for<zip_sentinel_t, zip_it_begin_t>);

	using zip_expected_type = subrange<zip_it_begin_t, zip_sentinel_t>;
	static_assert(forward_range<zip_expected_type>);

	auto zipped{ zip(array_t{0,1,2,3,4,5,6,7}) };
	static_assert(std::is_same_v<decltype(zipped), zip_expected_type>);

	return true;
}

bool czip_expected_behavior()
{
	using namespace globals;

	using zip_it_begin_t =
		forward_zip_iterator
		<
			array_cbegin_it_t,
			map_cbegin_it_t,
			list_cbegin_it_t,
			vector_cbegin_it_t,
			deque_cbegin_it_t
		>;
	static_assert(forward_iterator<zip_it_begin_t>);

	using zip_sentinel_t = typename zip_it_begin_t::difference_type;
	static_assert(std::sentinel_for<zip_sentinel_t, zip_it_begin_t>);

	using zip_expected_type = subrange<zip_it_begin_t, zip_sentinel_t>;
	static_assert(forward_range<zip_expected_type>);

	auto zipped{ czip(smallest_length, array, map, list, vector, deque) };
	static_assert(std::is_same_v<decltype(zipped), zip_expected_type>);

	static_assert(std::is_same_v<decltype(zipped.begin()), zip_it_begin_t>);
	const zip_it_begin_t begin_zip_it{
		array.cbegin(),
		map.cbegin(),
		list.cbegin(),
		vector.cbegin(),
		deque.cbegin()
	};
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	constexpr_assert(zipped.begin() == begin_zip_it);
	#pragma GCC diagnostic pop // ignored "-Wdeprecated-declarations"

	{
		auto auto_sized{ czip(array, map, list, vector, deque) };
		static_assert(std::is_same_v<decltype(zipped), decltype(auto_sized)>);
		#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
		constexpr_assert(zipped.begin() == auto_sized.begin());
		#pragma GCC diagnostic pop // ignored "-Wdeprecated-declarations"
		constexpr_assert(zipped.end() == auto_sized.end());
	}

	static_assert(std::is_same_v<decltype(zipped.end()), zip_sentinel_t>);
	const zip_sentinel_t end_zip_it{
		static_cast<zip_sentinel_t>(std::min({
			array.size(), map.size(), list.size(), vector.size(), deque.size()
		}))
	};
	constexpr_assert(zipped.end() == end_zip_it);

	array_begin_it_t array_it{ array.begin() };
	map_begin_it_t map_it{ map.begin() };
	list_begin_it_t list_it{ list.begin() };
	vector_begin_it_t vector_it{ vector.begin() };
	deque_begin_it_t deque_it{ deque.begin() };

	std::size_t loop_count{ 0 };
	for (const auto& [array_r, map_r, list_r, vector_r, deque_r] : zipped)
	{
		static_assert(!std::is_const_v<array_t>);
		static_assert(is_const_ref<decltype(array_r)>);
		constexpr_assert(&array_r == &*array_it++);

		static_assert(!std::is_const_v<map_t>);
		static_assert(is_const_ref<decltype(map_r)>);
		const auto& [map_k, map_v]{ map_r };
		const auto& [map_it_k, map_it_v]{ *map_it++ };
		constexpr_assert(map_k == map_it_k);
		constexpr_assert(&map_v == &map_it_v);

		static_assert(std::is_const_v<list_t>);
		static_assert(is_const_ref<decltype(list_r)>);
		constexpr_assert(&list_r == &*list_it++);

		static_assert(std::is_const_v<vector_t>);
		static_assert(is_const_ref<decltype(vector_r)>);
		constexpr_assert(&vector_r == &*vector_it++);

		static_assert(!std::is_const_v<deque_t>);
		static_assert(is_const_ref<decltype(deque_r)>);
		constexpr_assert(&deque_r == &*deque_it++);

		++loop_count;
	}
	constexpr_assert(loop_count == smallest_length);

	return true;
}

consteval bool czip_rvalues()
{

	using rval_begin_iter_t = decltype(std::cbegin(array_t{1,2,3,4,5,6,7}));
	using zip_it_begin_t = forward_zip_iterator<rval_begin_iter_t>;
	static_assert(forward_iterator<zip_it_begin_t>);

	using zip_sentinel_t = typename zip_it_begin_t::difference_type;
	static_assert(std::sentinel_for<zip_sentinel_t, zip_it_begin_t>);

	using zip_expected_type = subrange<zip_it_begin_t, zip_sentinel_t>;
	static_assert(forward_range<zip_expected_type>);

	auto zipped{ czip(array_t{0,1,2,3,4,5,6,7}) };
	static_assert(std::is_same_v<decltype(zipped), zip_expected_type>);

	return true;
}

bool test_passes()
{
	assert(zip_expected_behavior());
	assert(zip_rvalues());
	assert(czip_expected_behavior());
	assert(czip_rvalues());
	return true;
}

} // namespace forward::range

#endif // FGL_TEST_ZIP_FORWARD_RANGE_HPP_INCLUDED

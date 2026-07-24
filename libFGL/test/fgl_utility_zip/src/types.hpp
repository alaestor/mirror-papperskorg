#pragma once
#ifndef FGL_TEST_ZIP_TYPES_HPP_INCLUDED
#define FGL_TEST_ZIP_TYPES_HPP_INCLUDED

#include <array>
#include <vector>
#include <map>
#include <list>
#include <deque>

namespace container_and_range_types {

using array_t = std::array<short, 8>;
using map_t = std::map<int, char>;
using list_t = const std::list<double>;
using vector_t = const std::vector<char>;
using deque_t = std::deque<unsigned int>;

using array_begin_it_t = decltype(array_t().begin());
using map_begin_it_t = decltype(map_t().begin());
using list_begin_it_t = decltype(list_t().begin());
using vector_begin_it_t = decltype(vector_t().begin());
using deque_begin_it_t = decltype(deque_t().begin());

using array_cbegin_it_t = decltype(array_t().cbegin());
using map_cbegin_it_t = decltype(map_t().cbegin());
using list_cbegin_it_t = decltype(list_t().cbegin());
using vector_cbegin_it_t = decltype(vector_t().cbegin());
using deque_cbegin_it_t = decltype(deque_t().cbegin());

using array_end_it_t = decltype(array_t().end());
using map_end_it_t = decltype(map_t().end());
using list_end_it_t = decltype(list_t().end());
using vector_end_it_t = decltype(vector_t().end());
using deque_end_it_t = decltype(deque_t().end());

using array_cend_it_t = decltype(array_t().cend());
using map_cend_it_t = decltype(map_t().cend());
using list_cend_it_t = decltype(list_t().cend());
using vector_cend_it_t = decltype(vector_t().cend());
using deque_cend_it_t = decltype(deque_t().cend());

} // namespace container_and_range_types


#endif // FGL_TEST_ZIP_TYPES_HPP_INCLUDED

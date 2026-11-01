#pragma once
#ifndef FGL_UNITS_DATA_RATE_HPP_INCLUDED
#define FGL_UNITS_DATA_RATE_HPP_INCLUDED

#include <chrono>
#include "data_size.hpp"

namespace fgl::units {

template <typename T>
struct is_chrono_duration : public std::false_type
{};

template <typename T_rep, typename T_period>
struct is_chrono_duration<std::chrono::duration<T_rep, T_period>>
: public std::true_type
{};

template <typename T>
inline constexpr bool is_chrono_duration_v = is_chrono_duration<T>::value;

template <typename T>
concept chrono_duration_type = is_chrono_duration_v<T>;

template
<
	fgl::traits::numeric_type T_rep,
	data_size_type T_data_size,
	chrono_duration_type T_duration
>
struct data_rate
{
	using rep = T_rep;
	using data_size = T_data_size;
	using duration = T_duration;
	rep value;
	constexpr rep count() const { return value; }
};

using bits_per_second = data_rate<intmax_t, bits, std::chrono::seconds>;
using bytes_per_second = data_rate<intmax_t, bytes, std::chrono::seconds>;
using kilobits_per_second = data_rate<intmax_t, kilobits, std::chrono::seconds>;

} // namespace fgl::units

#endif // FGL_UNITS_DATA_RATE_HPP_INCLUDED

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <concepts> // same_as
#include <array>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>

#include <fgl/utility/matrix.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

/// TODO part of lib? common test utility? ...
template
<
    const std::size_t T_depth,
    const std::size_t T_length,
    const std::array<std::size_t, T_length> T_bounds
>
inline constexpr void iterate_bounds_impl(
    std::array<std::size_t, T_length>& indexes,
	const auto& callable)
{
    for (std::size_t& i{ indexes[T_depth] }; i < T_bounds[T_depth]; ++i)
    {
        if constexpr (T_depth < T_length-1)
        {
            iterate_bounds_impl<T_depth + 1, T_length, T_bounds>
				(indexes, callable);
        }
        else
        {
            callable(indexes);
        }
    }
    indexes[T_depth] = 0; // for next iteration
}

template <std::size_t ... T_bounds>
constexpr void iterate_bounds(const auto& callable)
{
    constexpr std::size_t dimensions{ sizeof...(T_bounds) };
    constexpr std::array<std::size_t, dimensions> bounds{ T_bounds... };
    std::array<std::size_t, dimensions> indexes{ 0 };
    iterate_bounds_impl<0, bounds.size(), bounds>(indexes, callable);
}
///

template <std::size_t ... T_bounds>
constexpr bool test_matrix()
{
	using T = unsigned int;
	using matrix_t = fgl::matrix<T, T_bounds...>;
	using base_t = matrix_t::base_t;
	using dimensional_t = matrix_t::dimensional_t;
	using expected_base_t = std::array<T, (T_bounds * ...)>;
	static_assert(std::same_as<base_t, expected_base_t>);

	matrix_t matrix;

	{
		T counter{};
		iterate_bounds<T_bounds...>(
			[&](const dimensional_t& indexes)
			{ matrix[indexes] = counter++; }
		);
	}

	{
		base_t expected;
		for (T counter{}; auto& v : expected)
			v = counter++;
		constexpr_assert(matrix == expected);
		matrix = expected;
	}

	static_assert(matrix_t::dimensions() == sizeof...(T_bounds));
	static_assert(matrix_t::bounds() == dimensional_t{T_bounds...});
	using fgl::linear_matrix_utilities::make_offsets;
	static_assert(
		make_offsets<matrix_t::dimensions()>(dimensional_t{T_bounds...})
			== matrix_t::offsets()
	);

	constexpr_assert(matrix.linear_at(0) == matrix.front());
	constexpr_assert(matrix.linear_at(0) == 0);
	constexpr_assert(matrix.linear_at(matrix.size()-1) == matrix.back());
	constexpr_assert(matrix.linear_at(matrix.size()-1) == matrix.size()-1);
	constexpr_assert(matrix.linear_index(0) == matrix.front());
	constexpr_assert(matrix.linear_index(0) == 0);
	constexpr_assert(matrix.linear_index(matrix.size()-1) == matrix.back());
	constexpr_assert(matrix.linear_index(matrix.size()-1) == matrix.size()-1);
	return true;
}

constexpr bool tests_pass()
{
	constexpr_assert((test_matrix<5,3,2>()));
	constexpr_assert((test_matrix<2,3,5>()));
	constexpr_assert((test_matrix<2,2,2>()));
	constexpr_assert((test_matrix<6,5,3,2>()));
	constexpr_assert((test_matrix<2,3,5,6>()));
	constexpr_assert((test_matrix<2,2,2,2>()));
	constexpr_assert((test_matrix<7,6,5,3,2>()));
	constexpr_assert((test_matrix<2,3,5,6,7>()));
	constexpr_assert((test_matrix<2,2,2,2,2>()));
	return true;
}

int main()
{
	static_assert(tests_pass());
	return EXIT_SUCCESS;
}

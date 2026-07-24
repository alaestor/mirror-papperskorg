#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <memory>

#define FGL_SHORT_MACROS
#include <fgl/debug/constexpr_assert.hpp>
#include <fgl/debug/exception_occurs.hpp>
#include <fgl/types/traits.hpp>

#include <fgl/types/not_null.hpp>

#ifdef NDEBUG
	#error NDEBUG must not be defined for tests because they rely on assertions
#endif // NDEBUG

/// @todo use misc.hpp same_address
[[nodiscard]] constexpr inline bool same_address(
	const void* const p1, const void* const p2) noexcept
{ return p1 == p2; }

constexpr bool test_not_null_interface()
{
	using T = int;
	T x{};

	// nonconst
	fgl::not_null nn{ &x };
	nn = &x;
	// unsafe reference
	static_assert(fgl::traits::is_nonconst_ref<decltype(nn.unsafe_reference())>);
	constexpr_assert(same_address(nn.unsafe_reference(), &x));
	// refernece
	static_assert(fgl::traits::is_const_ref<decltype(nn.reference())>);
	constexpr_assert(same_address(nn.reference(), &x));
	// copy
	static_assert(std::same_as<decltype(nn.copy()), T*>);
	constexpr_assert(same_address(nn.copy(), &x));
	// comparisons
	constexpr_assert(&*nn == &x);
	constexpr_assert(nn == nn);
	constexpr_assert(nn == &x);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	constexpr_assert(nn != nullptr);
#pragma GCC diagnostic pop

	// const
	const fgl::not_null cnn{ &x };
	// refernece
	static_assert(fgl::traits::is_const_ref<decltype(cnn.reference())>);
	constexpr_assert(same_address(cnn.reference(), &x));
	// copy
	static_assert(std::same_as<decltype(cnn.copy()), T*>);
	constexpr_assert(same_address(cnn.copy(), &x));
	// comparisons
	constexpr_assert(&*cnn == &x);
	constexpr_assert(cnn == cnn);
	constexpr_assert(cnn == &x);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	constexpr_assert(cnn != nullptr);
#pragma GCC diagnostic pop

	// indirect member access
	struct S { int member{}; };
	S s{};
	S* p_s{ &s };
	const fgl::not_null nn_s{ p_s };
	constexpr_assert(nn_s == p_s);
	constexpr_assert(same_address(&nn_s->member, &p_s->member));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	constexpr_assert(nn_s != nullptr);
#pragma GCC diagnostic pop

	return true;
}

/// @todo write test using std::unique_ptr with C++23
bool test_not_null_exceptions()
{
	struct toggle_null
	{
		bool am_i_null{ false };

		auto& operator*() noexcept
		{
			am_i_null = !am_i_null;
			return am_i_null;
		}

		// test equality with nullptr
		bool operator==(std::nullptr_t) const noexcept
		{ return am_i_null; }
	};

	fgl::not_null<toggle_null, false, true> nn;
	// strong guarantee
	assert(not exception_occurs((void)*nn));
	assert(specific_exception_occurs(std::runtime_error, (void)*nn));
	assert(specific_exception_occurs(std::runtime_error, (void)*nn));
	// access null object and change its state to be not-null
	nn.unsafe_reference().operator*();
	// now its not null again
	assert(not exception_occurs((void)*nn));

	toggle_null tn{ true };
	assert(specific_exception_occurs(std::invalid_argument, nn = tn));
	(void)*tn;
	assert(not exception_occurs(nn = tn));

	assert(specific_exception_occurs(
		std::invalid_argument,
		(void)fgl::not_null<int*>{nullptr})
	);

	int x{};
	fgl::not_null<int*> nn_x{ &x };
	assert(specific_exception_occurs(std::invalid_argument, (nn_x = nullptr)));

	return true;
}

int main()
{
	static_assert(test_not_null_interface());
	assert(test_not_null_interface());
	assert(test_not_null_exceptions());
	return EXIT_SUCCESS;
}

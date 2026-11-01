#pragma once
#ifndef FGL_TYPES_NOT_NULL_HPP_INCLUDED
#define FGL_TYPES_NOT_NULL_HPP_INCLUDED

#include <cstddef> // nullptr_t
#include <type_traits>
#include <concepts>
#include <utility> // forward, move
#include <stdexcept> // runtime_error, invalid_argument

#include "../debug/constexpr_assert.hpp"
#include "./traits.hpp" // null_comparable, pointer_type

namespace fgl {

/**
@file

@example example/fgl/types/not_null.cpp
	An example for @ref group-types-not_null

@defgroup group-types-not_null Not Null Wrapper

@brief A wrapper which ensures its value never has equality with @c nullptr .

@see the example program @ref example/fgl/types/not_null.cpp

@{
*/

/**
@copybrief group-types-not_null
@details @parblock
	This wrapper guarentees that its value will never have equality with
	@c nullptr by performing comparisons. How and where this not-null
	'contract' is enforced depends on the boolean template parameters.

	@remarks @parblock
	@c T& should be preferred over @c not_null<T*> where possible. However,
	there are many cases where @c not_null is an optimal solution, such
	as when interacting with legacy code, or when an interface requires
	something like @c fgl::not_null<std::unique_ptr<T>>

	<tt>@ref fgl::not_null</tt> serves two purposes: both improving code
	safety and documenting intent within the code itself. This can be
	compared to the popular @c gsl::not_null from the Core Guidelines
	Support Library. @c fgl::not_null is more flexible and configurable than
	the GSL : It can be made to have zero runtime overhead by using assertions
	instead of exceptions in cases where that is desirable. It can also be
	made to eliminate excessive "access" checks when they aren't required
	(such as with raw pointer types).

	Enforcement behavior is configured by boolean template parameters. This is
	a design choice which may be a bit controversial, as most libraries (like
	the GSL) use macro definitions to configure these behaviors. However, a
	type-based template approach was chosen to allow greater flexibility
	along with API / ABI stability. This allows different synthesizations to
	have different enforcement rules so that each instance can use whichever
	behavior is most applicable or efficient.
	@endparblock
@endparblock
@tparam T The <tt>@ref fgl::traits::null_comparable</tt> type to wrap
@tparam T_use_assertions If true, @ref group-debug-constexpr_assert will be
	used for enforcement instead of exceptions. This may be used to eliminate
	overhead when compiling for release with @c NDEBUG , but of course will
	also eliminate all @c nullptr checks. (Default: @c false , exceptions will
	be used)
@tparam T_check_on_access If true, perform @c nullptr checks upon access such
	as @c * , @c -> , @c copy() , and @c reference() . This can be used to
	eliminate redundant checks for types which can't change @c nullptr
	equality between accesses (Default:
	<tt>not @ref fgl::traits::pointer_type<T></tt> as these checks aren't
	beneficial for raw pointers).
*/
template
<
	fgl::traits::null_comparable T,
	const bool T_use_assertions = false,
	const bool T_check_on_access = not fgl::traits::pointer_type<T>
>
class not_null
{
	/// The wrapped object instance
	T m_value;

	/**
	@brief Checks that @p value isn't null
	@tparam T_except the type to throw if @p value has equality with @c nullptr
	@param value The value to check
	@throws T @parblock only when
	<ol>
	<li>@p T_use_assertions is @c false </li>
	<li>@p value has equality with @c nullptr </li>
	</ol>
	*/
	template <typename T_except>
	static constexpr void check_for_null(const T& value)
	noexcept(T_use_assertions)
	{
		if constexpr (T_use_assertions)
		{
			FGL_DEBUG_CONSTEXPR_ASSERT(value != nullptr);
		}
		else if (value == nullptr) throw T_except(
			"`fgl::not_null` has equality with `nullptr`"
		);
	}

	public:
	/// The type of the wrapped @p T object.
	using underlying_type = T;

	///@{ @name Constructors

	/**
	@throws std::invalid_argument @parblock only when
	<ol>
	<li>@p T_use_assertions is @c false </li>
	<li>The default-constructed @p T has equality with @c nullptr </li>
	</ol>
	Often, the "invalid argument" is in fact a <i>lack</i> of arguments.
	@endparblock
	*/
	[[nodiscard]] constexpr not_null()
	: m_value()
	{ check_for_null<std::invalid_argument>(m_value); }

	/**
	@param value A non-null value which will be move-constructed as the
		wrapped object.
	@throws std::invalid_argument @parblock only when
	<ol>
	<li>@p T_use_assertions is @c false </li>
	<li> @p value has equality with @c nullptr </li>
	</ol>
	@endparblock
	*/
	[[nodiscard]] constexpr not_null(T value)
	noexcept(T_use_assertions && std::is_nothrow_move_constructible_v<T>)
	: m_value(std::move(value))
	{ check_for_null<std::invalid_argument>(m_value); }

	/**
	@param[in] args Arguments to forward to the wrapped object's constructor.
	@throws std::invalid_argument @parblock only when
	<ol>
	<li>@p T_use_assertions is @c false </li>
	<li>the wrapped object has equality with @c nullptr </li>
	</ol>
	@endparblock
	*/
	[[nodiscard]] constexpr not_null(auto&& ... args)
	noexcept(
		T_use_assertions
		&& noexcept(T(std::forward<decltype(args)>(args)...))
	)
	: m_value(std::forward<decltype(args)>(args)...)
	{ check_for_null<std::invalid_argument>(m_value); }

	[[nodiscard]] constexpr not_null(const not_null& other)
	noexcept(std::is_nothrow_copy_constructible_v<T>)
	: m_value(other.m_value)
	{}

	///@}

	/**
	@returns a copy of the wrapped object
	@throws std::runtime_error @parblock only when
	<ol>
	<li>@p T_check_on_access is @c true </li>
	<li>@p T_use_assertions is @c false </li>
	<li>the wrapped object has equality with @c nullptr </li>
	</ol>
	@endparblock
	*/
	[[nodiscard]] constexpr T copy() const
	noexcept(
		std::is_nothrow_copy_constructible_v<T>
		&& (T_use_assertions || not T_check_on_access)
	)
	{
		if constexpr (T_check_on_access)
			check_for_null<std::runtime_error>(m_value);
		return T{ m_value };
	}

	/**
	@returns A constant reference to the wrapped object
	@throws std::runtime_error @parblock only when
	<ol>
	<li>@p T_check_on_access is @c true </li>
	<li>@p T_use_assertions is @c false </li>
	<li>the wrapped object has equality with @c nullptr </li>
	</ol>
	@endparblock
	*/
	[[nodiscard]] constexpr const T& reference() const
	noexcept(T_use_assertions || not T_check_on_access)
	{
		if constexpr (T_check_on_access)
			check_for_null<std::runtime_error>(m_value);
		return m_value;
	}

	/**
	@warning This method is unsafe for two reasons: it doesn't check for
		@c nullptr equality before returning the reference even when
		@p T_check_on_access is @c true , and because it can potentially be
		used to invalidate the 'not null' guarantee if the wrapped object is
		modified by reference to have equality with @c nullptr . The wrapper
		should be thought of as "dirty" if this method is invoked. This is
		particularly dangerous if @p T_check_on_access is @c false which can
		lead to invalid assumptions (i.e. when a @c not_null is actually null).
		However, if @p T_check_on_access is @c true then the guarantee can
		still be enforced by other access methods.
	@remarks It's strongly recommended that this method be avoided where
		possible, especially if @p T_check_on_access is @c false . This is
		provided for completeness and as a way to directly operate on the
		wrapped instance. For example: swapping or releasing a @c unique_ptr .
	@returns A reference to the wrapped object
	*/
	[[nodiscard]] constexpr T& unsafe_reference() noexcept
	{ return m_value; }

	constexpr not_null& operator=(const not_null& other)
	noexcept(std::is_nothrow_copy_assignable_v<T>)
	{
		m_value = other.m_value;
		return *this;
	}

	/**
	@returns The result of dereferencing the wrapped object.
	@throws std::runtime_error @parblock only when
	<ol>
	<li>@p T_check_on_access is @c true </li>
	<li>@p T_use_assertions is @c false </li>
	<li>the wrapped object has equality with @c nullptr </li>
	</ol>
	@endparblock
	*/
	[[nodiscard]] constexpr auto& operator*()
	noexcept(noexcept(*m_value) && (T_use_assertions || not T_check_on_access))
	{
		if constexpr (T_check_on_access)
			check_for_null<std::runtime_error>(m_value);
		return *m_value;
	}

	/**
	@returns The result of dereferencing the wrapped object.
	@throws std::runtime_error @parblock only when
	<ol>
	<li>@p T_check_on_access is @c true </li>
	<li>@p T_use_assertions is @c false </li>
	<li>the wrapped object has equality with @c nullptr </li>
	</ol>
	@endparblock
	*/
	[[nodiscard]] constexpr auto& operator*() const
	noexcept(noexcept(*m_value) && (T_use_assertions || not T_check_on_access))
	{
		if constexpr (T_check_on_access)
			check_for_null<std::runtime_error>(m_value);
		return *m_value;
	}

	/**
	@throws std::runtime_error @parblock only when
	<ol>
	<li>@p T_check_on_access is @c true </li>
	<li>@p T_use_assertions is @c false </li>
	<li>the wrapped object has equality with @c nullptr </li>
	</ol>
	@endparblock
	*/
	[[nodiscard]] constexpr const auto& operator->()
	noexcept(T_use_assertions || not T_check_on_access)
	{
		if constexpr (T_check_on_access)
			check_for_null<std::runtime_error>(m_value);
		return m_value;
	}

	/**
	@throws std::runtime_error @parblock only when
	<ol>
	<li>@p T_check_on_access is @c true </li>
	<li>@p T_use_assertions is @c false </li>
	<li>the wrapped object has equality with @c nullptr </li>
	</ol>
	@endparblock
	*/
	[[nodiscard]] constexpr const auto& operator->() const
	noexcept(T_use_assertions || not T_check_on_access)
	{
		if constexpr (T_check_on_access)
			check_for_null<std::runtime_error>(m_value);
		return m_value;
	}

	///@{ @relates fgl::not_null @name Comparison operators

	[[deprecated(
		"fgl::not_null should never have equality with nullptr. "
		"This comparison operator exists to fascilitate refactoring. "
		"If you really intend to compare the wrapped object, use reference()"
	)]]
	[[nodiscard]] friend inline constexpr
	bool operator==(const not_null& lhs, const std::nullptr_t rhs) noexcept
	{ return lhs.m_value == rhs; }

	[[nodiscard]] friend inline constexpr
	bool operator==(const not_null& lhs, const not_null& rhs)
	noexcept(noexcept(lhs.m_value == rhs.m_value))
	{ return lhs.m_value == rhs.m_value; }

	[[nodiscard]] friend inline constexpr
	auto operator<=>(const not_null& lhs, const not_null& rhs)
	noexcept(noexcept(lhs.m_value == rhs.m_value))
	{ return lhs.m_value <=> rhs.m_value; }

	///@}
};

/// A helper alias to improve code readability & expressiveness
template
<
	fgl::traits::null_comparable T,
	const bool T_check_on_access = not fgl::traits::pointer_type<T>
>
using assert_not_null = fgl::not_null<T, true, T_check_on_access>;

///@}
}; // namespace fgl

#endif // FGL_TYPES_NOT_NULL_HPP_INCLUDED

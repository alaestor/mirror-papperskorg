#pragma once
#ifndef FGL_TYPES_UNIQUE_HPP_INCLUDED
#define FGL_TYPES_UNIQUE_HPP_INCLUDED

/// QUICK-START GUIDE
/*
*/
/// EXAMPLE PROGRAM
/*
*/

#include <utility> // forward

#ifdef FGL_SHORT_MACROS
	#define FGL_TYPES_UNIQUE_SHORT_MACROS
#endif

#ifndef FGL_TYPES_DECLARE_WRAPPER_STRUCT
///////////////////////////////////////////////////////////////////////////////
	#define FGL_TYPES_DECLARE_WRAPPER_STRUCT(name, type) \
		struct name\
		{\
			using aliased_type = type;\
			aliased_type object;\
			[[nodiscard]] constexpr name(auto&& ... args)\
			: object(std::forward<decltype(args)>(args) ...)\
			{}\
			[[nodiscard]] constexpr explicit name(const name&) = default;\
			[[nodiscard]] constexpr explicit name(name&&) = default;\
			[[nodiscard]] constexpr name& operator=(const name&) = default;\
			[[nodiscard]] constexpr name& operator=(name&&) = default;\
			[[nodiscard]] constexpr aliased_type& get() noexcept { return object; }\
			[[nodiscard]] constexpr const aliased_type& get() const noexcept { return object; }\
			[[nodiscard]] constexpr aliased_type get_copy() const { return object; }\
			[[nodiscard]] constexpr const aliased_type& get_const() const noexcept { return object; }\
			[[nodiscard]] constexpr aliased_type& operator()() noexcept { return object; }\
			[[nodiscard]] constexpr const aliased_type& operator()() const noexcept { return object; }\
			[[nodiscard]] constexpr operator aliased_type&() noexcept { return object; }\
			[[nodiscard]] constexpr operator const aliased_type&() const noexcept { return object; }\
			[[nodiscard]] constexpr bool operator==(const name&) const = default;\
			[[nodiscard]] constexpr auto operator<=>(const name&) const = default;\
			void operator+(const aliased_type&) = delete;\
			void operator-(const aliased_type&) = delete;\
			void operator*(const aliased_type&) = delete;\
			void operator/(const aliased_type&) = delete;\
			void operator=(const aliased_type&) = delete;\
			bool operator==(const aliased_type&) = delete;\
			auto operator<=>(const aliased_type&) = delete;\
		}
///////////////////////////////////////////////////////////////////////////////
#else
	#error FGL_TYPES_DECLARE_WRAPPER_STRUCT already defined
#endif // ifndef FGL_TYPES_DECLARE_WRAPPER_STRUCT

#ifdef FGL_TYPES_WRAPPER_STRUCT_SHORT_MACROS
	#ifndef DECLARE_WRAPPER_STRUCT
///////////////////////////////////////////////////////////////////////////////
		#define DECLARE_WRAPPER_STRUCT(name, type) \
			FGL_TYPES_DECLARE_WRAPPER_STRUCT(name, type)
///////////////////////////////////////////////////////////////////////////////
	#else
		#error DECLARE_WRAPPER_STRUCT already defined
	#endif
#endif // FGL_TYPES_UNIQUE_SHORT_MACROS

/*
#ifndef FGL_TYPES_DECLARE_WRAPPER_STRUCT
///////////////////////////////////////////////////////////////////////////////
	#define FGL_TYPES_DECLARE_WRAPPER_STRUCT(name, type) \
		[[maybe_unused]] static constexpr inline \
			auto name ## _fingerprint{[](){}}; \
		using name = typename \
			fgl::unique<type, decltype(name ## _fingerprint)>
///////////////////////////////////////////////////////////////////////////////
#else
	#error FGL_TYPES_DECLARE_WRAPPER_STRUCT already defined
#endif // ifndef FGL_TYPES_DECLARE_WRAPPER_STRUCT

#ifdef FGL_TYPES_UNIQUE_SHORT_MACROS
	#ifndef DECLARE_UNIQUE_ALIAS
///////////////////////////////////////////////////////////////////////////////
		#define DECLARE_UNIQUE_ALIAS(name, type) \
			FGL_TYPES_DECLARE_WRAPPER_STRUCT(name, type)
///////////////////////////////////////////////////////////////////////////////
	#else
		#error DECLARE_UNIQUE_ALIAS already defined
	#endif
#endif // FGL_TYPES_UNIQUE_SHORT_MACROS

namespace fgl {

// A unique type template. By default, each instantiation is a unique type.
template <class T, typename T_fingerprint = decltype([](){})>
struct unique
{
	using fingerprint = T_fingerprint;
	using aliased_type = T;
	T object;

	[[nodiscard]] constexpr T& get() noexcept
	{ return object; }

	[[nodiscard]] constexpr const T& get() const noexcept
	{ return object; }

	[[nodiscard]] constexpr T get_copy() const
	{ return object; }

	[[nodiscard]] constexpr const T& get_const() const noexcept
	{ return object; }

	[[nodiscard]] constexpr T& operator()() noexcept
	{ return object; }

	[[nodiscard]] constexpr const T& operator()() const noexcept
	{ return object; }

	void operator+(const T&) = delete;
	void operator-(const T&) = delete;
	void operator*(const T&) = delete;
	void operator/(const T&) = delete;
	void operator=(const T&) = delete;
	bool operator==(const T&) = delete;
	auto operator<=>(const T&) = delete;

	auto operator<=>(const unique&) const = default;
};

} // namespace fgl
*/

#endif // FGL_TYPES_UNIQUE_HPP_INCLUDED

#pragma once
#ifndef FGL_TYPES_RANGE_CONSTRAINTS_HPP_INCLUDED
#define FGL_TYPES_RANGE_CONSTRAINTS_HPP_INCLUDED
#include "../environment/libfgl_compatibility_check.hpp"

#include <concepts>
#include <ranges>

#include "./traits.hpp"

///@cond FGL_INTERNAL_DOCS
#ifndef FGL_INTERNAL_CR_COMPARE
	/**
	@internal
	@brief Codegen for creating conjunctions two concepts conjunctions;
		the regular comparison and <tt>_range</tt> variant
	*/
	#define FGL_INTERNAL_CR_COMPARE(ns1, t1, ns2, t2, ns3, t3)\
		template <class T_range, class T_compare>\
		concept t1 ## _ ## t2 = ns1::t1<T_range>\
			&& ns2::t2<ns3::t3<T_range>, T_compare>;\
		template <class T_range, class T_compare_range>\
		concept t1 ## _ ## t2 ## _range = ns1::t1<T_range>\
			&& ns2::t2<ns3::t3<T_range>, ns3::t3<T_compare_range>>;
#else
	#error FGL_INTERNAL_CR_COMPARE already defined
#endif

#ifndef FGL_INTERNAL_CR_COMPARE_V
	///@internal @brief Codegen; compares std::ranges with range_value_t
	#define FGL_INTERNAL_CR_COMPARE_V(ns1, t1, ns2, t2)\
		FGL_INTERNAL_CR_COMPARE(ns1, t1, ns2, t2, std::ranges, range_value_t)
#else
	#error FGL_INTERNAL_CR_COMPARE_V already defined
#endif

#ifndef FGL_INTERNAL_CR_COMPARE_R
	///@internal @brief Codegen; compares std::ranges with range_reference_t
	#define FGL_INTERNAL_CR_COMPARE_R(ns1, t1, ns2, t2)\
		FGL_INTERNAL_CR_COMPARE(\
			ns1, t1, ns2, t2, std::ranges, range_reference_t)
#else
	#error FGL_INTERNAL_CR_COMPARE_R already defined
#endif

#ifndef FGL_INTERNAL_CR_CHECK
	///@internal @brief Codegen; conjunction, no comparison
	#define FGL_INTERNAL_CR_CHECK(ns1, t1, ns2, t2, ns3, t3)\
		template <class T_range>\
		concept t1 ## _ ## t2 = ns1::t1<T_range>\
			&& ns2::t2<ns3::t3<T_range>>;
#else
	#error FGL_INTERNAL_CR_CHECK already defined
#endif

#ifndef FGL_INTERNAL_CR_CHECK_V
	///@internal @brief Codegen; checks std::ranges with range_value_t
	#define FGL_INTERNAL_CR_CHECK_V(ns1, t1, ns2, t2)\
		FGL_INTERNAL_CR_CHECK(ns1, t1, ns2, t2, std::ranges, range_value_t)
#else
	#error FGL_INTERNAL_CR_CHECK_V already defined
#endif

#ifndef FGL_INTERNAL_CR_CHECK_R
	///@internal @brief Codegen; checks std::ranges with range_reference_t
	#define FGL_INTERNAL_CR_CHECK_R(ns1, t1, ns2, t2)\
		FGL_INTERNAL_CR_CHECK(ns1, t1, ns2, t2, std::ranges, range_reference_t)
#else
	#error FGL_INTERNAL_CR_CHECK_R already defined
#endif

#ifndef FGL_INTERNAL_CR_GENERATE_PERMUTATIONS
	///@internal @brief Codegen; generates concepts for all supported concepts
	#define FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(ns, range)\
	/* STD */\
	FGL_INTERNAL_CR_COMPARE_V(ns, range, std, same_as);\
	FGL_INTERNAL_CR_COMPARE_V(ns, range, std, derived_from);\
	FGL_INTERNAL_CR_COMPARE_V(ns, range, std, convertible_to);\
	FGL_INTERNAL_CR_COMPARE_V(ns, range, std, common_with);\
	FGL_INTERNAL_CR_COMPARE_R(ns, range, std, swappable_with);\
	FGL_INTERNAL_CR_COMPARE_R(ns, range, std, assignable_from);\
	FGL_INTERNAL_CR_CHECK_V(ns, range, std, integral);\
	FGL_INTERNAL_CR_CHECK_V(ns, range, std, signed_integral);\
	FGL_INTERNAL_CR_CHECK_V(ns, range, std, unsigned_integral);\
	FGL_INTERNAL_CR_CHECK_V(ns, range, std, floating_point);\
	/* FGL */\
	FGL_INTERNAL_CR_COMPARE_V(ns, range, fgl::traits, not_same_as);\
	FGL_INTERNAL_CR_CHECK_V(ns, range, fgl::traits, numeric_type);\
	FGL_INTERNAL_CR_CHECK_V(ns, range, fgl::traits, byte_type);\
	FGL_INTERNAL_CR_CHECK_V(ns, range, fgl::traits, pointer_to_byte);\
	FGL_INTERNAL_CR_CHECK_V(ns, range, fgl::traits, pointer_to_non_void);
#else
	#error FGL_INTERNAL_CR_GENERATE_PERMUTATIONS already defined
#endif

namespace fgl {
/**
@{ @name
@internal
@brief Codegen; generates concept permutations for all supported ranges
*/
FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(std::ranges, range);
FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(std::ranges, borrowed_range);
FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(std::ranges, sized_range);
FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(std::ranges, forward_range);
FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(std::ranges, bidirectional_range);
FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(std::ranges, random_access_range);
FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(std::ranges, contiguous_range);
FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(std::ranges, common_range);
FGL_INTERNAL_CR_GENERATE_PERMUTATIONS(std::ranges, viewable_range);
///@}
} // namespace fgl
///@endcond FGL_INTERNAL_DOCS

/**
@file

@example example/fgl/types/range_constraints.cpp
	An example for @ref group-types-range_constraints

@defgroup group-types-range_constraints Range Constraints

@brief Concepts to express constraining range and underlying types

@remarks
	Firstly, I'd like to apologize for the lack of direct documentation.
	Due to the sheer number of permutations possible between ranges and type
	constraints, this header makes use of internal macros to generate all the
	concepts. Because of this, doxygen can't generate the documentation for
	the individual concepts. Ideally, this would be better expressed as a set
	of concept templates; however, the C++ standard does not allow for this as
	concepts cannot be template arguments. There are work-arounds, but their
	semmantics are just as if not more noisy than the normal boiler-plate,
	and the error messages are often indecipherable or lack critical
	information, such as why the concept was not satisfied. Please bear with
	with me for this explanation. Once you understand the syntax, they are
	quite intuitive to use.

@details @parblock
	<tt>#include <fgl/types/@ref range_constraints.hpp></tt> defines
	concepts that allow for constraining ranges based on the value or
	reference type without the <tt>template<> requires</tt> boiler-plate noise.

	Range constraints are conjunctions between a range concept and a type
	concept which constrains either the <tt>std::ranges::range_value_t</tt> or
	<tt>std::ranges::range_reference_t</tt>, depending on which is most
	applicable to the concept.

	<b>Generated concept syntax:</b>
	<ul>
	<li>
		A conjunction between a range and type concept
		<blockquote>
		<tt>template <T_range> concept
		fgl::</tt><i>RANGE CONCEPT NAME</i><tt>_</tt><i>TYPE CONCEPT NAME</i>
		</blockquote>
		@tparam T_range the range being tested
		@param "RANGE CONCEPT NAME" the name of the concept which constrains
			the range, such as <tt>contiguous_range</tt>
		@param "TYPE CONCEPT NAME" the name of the concept which constrains
			the type, such as <tt>integral</tt>

		So if you wanted to test if a range was a contiguous range of
		integrals, you would write
		<tt>fgl::contiguous_range_integral<std::vector<int>></tt>.
	</li>



	<li>
		A conjunction between a range and a type concept which requires a
		comparison to another type, such as <tt>std::same_as</tt>
		<blockquote>
		<tt>template <T_range, T_compare> concept
		fgl::</tt><i>RANGE CONCEPT NAME</i><tt>_</tt><i>TYPE CONCEPT NAME</i>
		</blockquote>
		@tparam T_range the range being tested
		@tparam T_compare the type being compared to
		@param "RANGE CONCEPT NAME" the name of the concept which constrains
			the range, such as <tt>forward_range</tt>
		@param "TYPE CONCEPT NAME" the name of the concept which constrains
			the type, such as <tt>same_as</tt>

		So if you wanted to test if a range was a forward range of
		<tt>int</tt>s, you would write
		<tt>fgl::forward_range_same_as<std::list<int>, int></tt>.
	</li>



	<li>
		A conjunction between a range and a type concept which requires a
		comparison to the underlying type of another range, such as
		<tt>std::same_as<std::ranges::range_value_t<T_compare_range>></tt> or
		<tt>std::same_as<std::ranges::range_reference_t<T_compare_range>></tt>
		<blockquote>
		<tt>template <T_range, T_compare_range> concept
		fgl::</tt><i>RANGE CONCEPT NAME</i><tt>_</tt>
		<i>TYPE CONCEPT NAME</i><tt>_range</tt>
		</blockquote>
		@tparam T_range the range being tested
		@tparam T_compare_range the range whose type is being compared to
		@param "RANGE CONCEPT NAME" the name of the concept which constrains
			the range, such as <tt>bidirectional_range</tt>
		@param "TYPE CONCEPT NAME" the name of the concept which constrains
			the type, such as <tt>same_as</tt>

		So if you wanted to test if a range was a bi-directional range whose
		underlying type was the same as the type of another range, you would
		write <tt>fgl::bidirectional_range_same_as_range<std::array<int, 3>,
		std::vector<int>></tt>.
	</ul>


	<b>Supported range concepts:</b>

	- std::ranges::<tt>range</tt>
	- std::ranges::<tt>borrowed_range</tt>
	- std::ranges::<tt>sized_range</tt>
	- std::ranges::<tt>bidirectional_range</tt>
	- std::ranges::<tt>random_access_range</tt>
	- std::ranges::<tt>contiguous_range</tt>
	- std::ranges::<tt>common_range</tt>
	- std::ranges::<tt>viewable_range</tt>

	<b>Supported type concepts:</b>

	- std::<tt>swappable_with<T&></tt>
	- std::<tt>assignable_from<T></tt>
	- std::<tt>same_as<T></tt>
	- std::<tt>derived_from<T></tt>
	- std::<tt>convertible_to<T></tt>
	- std::<tt>common_with<T></tt>
	- std::<tt>integral</tt>
	- std::<tt>signed_integral</tt>
	- std::<tt>unsigned_integral</tt>
	- std::<tt>floating_point</tt>
	- fgl::traits::<tt>not_same_as<T></tt>
	- fgl::traits::<tt>numeric_type</tt>
	- fgl::traits::<tt>byte_type</tt>
	- fgl::traits::<tt>pointer_to_byte</tt>
	- fgl::traits::<tt>pointer_to_non_void</tt>

	Refer to libFGL's @ref group-types-traits for the
	<tt>fgl::traits</tt> concepts

	<b>Type concepts and the <tt>_range</tt> suffix:</b>

	Each comparison concept also has a variant with the <tt>_range</tt> suffix,
	whose template arguments are <tt><T_range, T_compare_range></tt>. This
	variant will retrieve the constraint comparison type from
	<tt>T_compare_range</tt>'s value or reference type; which ever is used to
	retrieve the type being constrained from <tt>T_range</tt>.
@endparblock
@see the example program @ref example/fgl/types/range_constraints.cpp
*/

#endif // FGL_TYPES_RANGE_CONSTRAINTS_HPP_INCLUDED

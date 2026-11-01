/// Concept helpers

/* NOT ADVISED!
Diagnostics won't tell you which concept failed :(
This hack exists because concepts can't be template params.*/
template <auto T_concept_lambda, typename T>
inline constexpr bool satisfies_concept_lambda{
	T_concept_lambda.template operator()<T>()
};

template <auto T_concept_lambda, typename T>
concept for_each_cv_permutation =
	satisfies_concept_lambda<T_concept_lambda, T>
	&& satisfies_concept_lambda<T_concept_lambda, T const>
	&& satisfies_concept_lambda<T_concept_lambda, T volatile>
	&& satisfies_concept_lambda<T_concept_lambda, T const volatile>
;

template <auto T_concept_lambda, typename T>
concept for_each_cvptr_permutation =
	for_each_cv_permutation<T_concept_lambda, T*>
	&& for_each_cv_permutation<T_concept_lambda, const T*>
	&& for_each_cv_permutation<T_concept_lambda, volatile T*>
	&& for_each_cv_permutation<T_concept_lambda, const volatile T*>
;

/*
example:

constexpr auto pointer_to_byte_concept_lambda{
	[]<typename T>() consteval { return pointer_to_byte<T>; }
};

template <typename T>
concept can_be_byte_pointer_type =
	for_each_cvptr_permutation<pointer_to_byte_concept_lambda, T>;

static_assert(can_be_byte_pointer_type<std::byte>);
static_assert(!can_be_byte_pointer_type<int>);

// is like doing:
//		concept<T*>
//		&& concept<T* const>
//		&& concept<T* volatile>
//		&& concept<T* const volatile>
//		&& concept<const T*>
//		&& concept<const T* const>
//		&& concept<const T* volatile>
//		&& concept<const T* const volatile>
//		&& concept<volatile T*>
//		&& concept<volatile T* const>
//		&& concept<volatile T* volatile>
//		&& concept<volatile T* const volatile>
//		&& concept<const volatile T*>
//		&& concept<const volatile T* const>
//		&& concept<const volatile T* volatile>
//		&& concept<const volatile T* const volatile>
*/

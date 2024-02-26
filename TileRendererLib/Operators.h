#pragma once
#include <type_traits>

//https://stackoverflow.com/questions/28309164/checking-for-existence-of-an-overloaded-member-function
namespace operators {

	template<class t, class...args>
	using test_operator_equals = decltype(
		std::declval<t>().operator==(std::declval<args>()...));

	template<class t, class sig, class = void>
	struct has_operator_equals :std::false_type {};

	template<class t, class...args>
	struct has_operator_equals<t, void(args...),
		std::void_t<
		test_operator_equals<t, args...>
		>
	> :std::true_type {};

	template<class t, class r, class...args>
	struct has_operator_equals <t, r(args...),
		typename std::enable_if<
		!std::is_same<r, void>::value&&
		std::is_convertible<
		test_operator_equals<t, args...>,
		r
		>::value
		>::type
	> :std::true_type {};

}
// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"

DBJ_TEST_SPACE_OPEN(dbj_traits_testing)

template<typename T, typename dbj::require_integral<T> = 0 >
inline auto Object(T&& t) { return std::variant<T>(t); }

template<typename T, typename dbj::require_floating<T> = 0>
inline auto Object(T&& t) { return std::variant<T>(t); }

/*usage*/
DBJ_TEST_UNIT(_dbj_basic_traits_tests) {

	DBJ_TEST_ATOM(dbj::is_floating<decltype(42.0f)>());
	DBJ_TEST_ATOM(dbj::is_integral<decltype(42u)>());
	DBJ_TEST_ATOM(dbj::is_object<decltype(42u)>());

	DBJ_TEST_ATOM(Object(42));
	DBJ_TEST_ATOM(Object(42.0f));
}

DBJ_TEST_UNIT( _dbj_container_traits_tests )
{
	using namespace std;

	using ia3 = array<int, 3>;
	using vi = vector<int>;

	DBJ_TEST_ATOM(dbj::is_std_array_v<ia3>);
	DBJ_TEST_ATOM(dbj::is_std_array_v<vi>);

	DBJ_TEST_ATOM(dbj::is_std_vector_v<ia3>);
	DBJ_TEST_ATOM(dbj::is_std_vector_v<vi>);

	DBJ_TEST_ATOM(dbj::is_range_v<ia3>);
	DBJ_TEST_ATOM(dbj::is_range_v<vi>);

	// bellow wont work because comma operator screws macros in a royal way
	// DBJ_TEST_ATOM(dbj::inner::is_range<range_container<int,3>>::value);
	// it actually screws the whole closure it is in
	// https://stackoverflow.com/questions/13842468/comma-in-c-c-macro
	// ditto ...

	using rci3 = dbj::util::rac<int, 3>;
	DBJ_TEST_ATOM(dbj::is_range_v<rci3>);
}

DBJ_TEST_UNIT(_dbj_pointer_traits_tests) {
	using namespace dbj::tt;
	constexpr const char * holla_ = "Hola!";
	const char buff[]{ "ABCD" };
	// OK
	static_assert(pointer(buff));
	static_assert(pointer(holla_));
	static_assert(pointer("ola ola!"));
	//
	const std::array<int, 3> iarr{ 1,2,3 };
	static_assert(pointer(iarr.data()));

	auto n1 = DBJ_TEST_ATOM( DBJ_VALTYPENAME(holla_ ));
	auto n2 = DBJ_TEST_ATOM( DBJ_VALTYPENAME(&holla_));
}

DBJ_TEST_SPACE_CLOSE


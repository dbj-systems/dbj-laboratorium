#pragma once

#include <deque>
#include <list>
#include <forward_list>

DBJ_TEST_SPACE_OPEN(string_util_tests )


#pragma region dbj strlen and strnlen
/*
dbj crt caters for char, wchar_t, char16_t, char32_t
for details please see https://docs.microsoft.com/en-us/cpp/cpp/char-wchar-t-char16-t-char32-t
*/
template< typename T, size_t N>
inline void strlen_strnlen_test
(
	 const T (&prompt)[N]
)
{
	using ::dbj::console::print;
	print("\n\nTesting array of type ", typeid(T).name(), " and of length ", N, "\t");
	DBJ_ATOM_TEST(dbj::countof(prompt));
	// native char arrays are using dbj.org "zero time" versions     
	DBJ_ATOM_TEST(dbj::str::strlen( prompt ));
	DBJ_ATOM_TEST(dbj::str::strnlen(prompt, dbj::BUFSIZ_));

	// testing for the T * support 
	auto pointer_tester = [&](T const * cptr)
	{
		using ::dbj::console::print;
		// cptr become a pointer due to the standard decay
		using pointer_to_array = decltype(cptr);

		print("\n\nTesting the support for the ", typeid(pointer_to_array).name(), " pointer to the same array\n");
		
		// using UCRT strlen
		DBJ_ATOM_TEST(::dbj::str::strlen( cptr ));

		// using UCRT strnlen note: std has no strnlen ...
		DBJ_ATOM_TEST(::dbj::str::strnlen(cptr, dbj::BUFSIZ_));
	};

	pointer_tester(prompt);
}

DBJ_TEST_UNIT(dbj_strnlen)
{
	using ::dbj::console::print;

	constexpr char	 promptA[] = "0123456789";
	constexpr wchar_t  promptW[] = L"0123456789";
	constexpr char16_t prompt16[] = u"0123456789";
	constexpr char32_t prompt32[] = U"0123456789";

	strlen_strnlen_test(promptA);
	strlen_strnlen_test(promptW);
	strlen_strnlen_test(prompt16);
	strlen_strnlen_test(prompt32);

}

#pragma endregion 

// todo: pull in the C solution from dbj clib
#ifdef DBJ_USE_STD_STREAMS
DBJ_TEST_UNIT(dbjstrtokenizer)
{
	DBJ_ATOM_TEST(dbj::str::tokenize("prefix mif fix fenix"));
	DBJ_ATOM_TEST(dbj::str::tokenize(L"prefix mif fix fenix"));
}
#endif

DBJ_TEST_UNIT(dbjstroptimal) {

	// capacity and size of os1 is 255
	// it will not do any heap alloc / de-alloc
	// for size < 255
	DBJ_ATOM_TEST(dbj::str::optimal<char>());
	// increase to 1024
	DBJ_ATOM_TEST(dbj::str::optimal<wchar_t>(1024));
	// increase to 512 and stamp with u'='
	DBJ_ATOM_TEST(dbj::str::optimal<char16_t>(512, u'='));
	// increase to 128 and stamp with U'+'
	DBJ_ATOM_TEST(dbj::str::optimal<char32_t>(128, U'+'));
}



DBJ_TEST_UNIT(dbjstringutils) {
	//auto rez =
		DBJ_ATOM_TEST(
			0 == ::dbj::dbj_ordinal_string_compareW(L"A", L"A", true));
	//auto zer =
		DBJ_ATOM_TEST(
			dbj::str::ui_string_compare(L"abra babra dabra", L"ABRA babra DABRA", true));

	//auto rez_2 = 
		DBJ_ATOM_TEST(
		dbj::str::is_prefix(L"PREFIX", L"PREFIX MIX FIX"));

	//auto rez_3 = 
		DBJ_ATOM_TEST(
		dbj::str::is_prefix(L"MINIX", L"PREFIX MIX FIX"));

	
	//this will not compile, because one
	//can not lowerize string literals
	//auto rez_4 = DBJ_ATOM_TEST(dbj::str::lowerize(L"PREFIX MIX FIX" ));
	

	//auto rez_4 = 
	   DBJ_ATOM_TEST(dbj::str::lowerize(L"PREFIX MIX FIX"sv));

}


DBJ_TEST_SPACE_CLOSE

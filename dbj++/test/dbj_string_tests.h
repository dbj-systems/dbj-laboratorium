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
	const T(&prompt)[N]
)
{
	using ::dbj::console::print;
	print("\n\nTesting array of type ", typeid(T).name(), " and of length ", N, "\t");
	DBJ_ATOM_TEST(dbj::countof(prompt));
	// native char arrays are using dbj.org "zero time" versions     
	DBJ_ATOM_TEST(dbj::str::strlen(prompt));
	DBJ_ATOM_TEST(dbj::str::strnlen(prompt, dbj::BUFSIZ_));

	// testing for the T * support 
	auto pointer_tester = [&](auto cptr)
	{
		using ::dbj::console::print;
		// cptr become a pointer due to the standard decay
		using pointer_to_array = decltype(cptr);

		print("\n\nTesting the support for the ", typeid(pointer_to_array).name(), " pointer to the same array\n");
		// using UCRT strlen
		DBJ_ATOM_TEST(dbj::str::strlen(cptr));

		// using UCRT strnlen note: std has no strnlen ...
		DBJ_ATOM_TEST(dbj::str::strnlen(cptr, dbj::BUFSIZ_));
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

	static_assert(dbj::str::strnlen(promptA, dbj::BUFSIZ_) == 10);
	static_assert(dbj::str::strnlen(promptW, dbj::BUFSIZ_) == 10);
	static_assert(dbj::str::strnlen(prompt16, dbj::BUFSIZ_) == 10);
	static_assert(dbj::str::strnlen(prompt32, dbj::BUFSIZ_) == 10);

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
	

	wchar_t wide_charr_arr[]{ L"PREFIX MIX FIX" };
	//auto rez_4 = 
	   DBJ_ATOM_TEST(dbj::str::lowerize(wide_charr_arr));

}


/// <summary>
///  the string meta converter testing
/// </summary>
template<typename converter_type>
inline void test_conversion(converter_type && the_converter)
{
	using namespace std::string_literals;
	auto arf = std::ref(L"Hello!");

	// DBJ_ATOM_TEST(the_converter);

	// standard string literals
	DBJ_ATOM_TEST(the_converter("\"the\0\0standard string literal\""s));
	DBJ_ATOM_TEST(the_converter(L"\"the\0\0standard string literal\""s));
	DBJ_ATOM_TEST(the_converter(u"\"the\0\0standard string literal\""s));
	DBJ_ATOM_TEST(the_converter(U"\"the\0\0standard string literal\""s));

	// C string literals
	DBJ_ATOM_TEST(the_converter("\"Abra Ca Dabra Alhambra\""));
	DBJ_ATOM_TEST(the_converter(L"\"Abra Ca Dabra Alhambra\""));
	DBJ_ATOM_TEST(the_converter(u"\"Abra Ca Dabra Alhambra\""));
	DBJ_ATOM_TEST(the_converter(U"\"Abra Ca Dabra Alhambra\""));

	// the four standard string types
	DBJ_ATOM_TEST(the_converter(std::string{ "\"Abra Ca Dabra Alhambra\"" }));
	DBJ_ATOM_TEST(the_converter(std::wstring{ L"\"Abra Ca Dabra Alhambra\"" }));
	DBJ_ATOM_TEST(the_converter(std::u16string{ u"\"Abra Ca Dabra Alhambra\"" }));
	DBJ_ATOM_TEST(the_converter(std::u32string{ U"\"Abra Ca Dabra Alhambra\"" }));

	// now let's try the sequence containers

	// the four standard char types in std::array
	DBJ_ATOM_TEST(the_converter(std::array<char, 6>{  'H', 'e', 'l', 'l', 'o', '!'}));
	DBJ_ATOM_TEST(the_converter(std::array<wchar_t, 6>{ L'H', L'e', L'l', L'l', L'o', L'!'}));
	DBJ_ATOM_TEST(the_converter(std::array<char16_t, 6>{ u'H', u'e', u'l', u'l', u'o', u'!'}));
	DBJ_ATOM_TEST(the_converter(std::array<char32_t, 6>{ U'H', U'e', U'l', U'l', U'o', U'!'}));

	// the four standard char types in std::vector
	DBJ_ATOM_TEST(the_converter(std::vector<char>	{  'H', 'e', 'l', 'l', 'o', '!'}));
	DBJ_ATOM_TEST(the_converter(std::vector<wchar_t>	{ L'H', L'e', L'l', L'l', L'o', L'!'}));
	DBJ_ATOM_TEST(the_converter(std::vector<char16_t>{ u'H', u'e', u'l', u'l', u'o', u'!'}));
	DBJ_ATOM_TEST(the_converter(std::vector<char32_t>{ U'H', U'e', U'l', U'l', U'o', U'!'}));

	// the four standard char types in std::deque
	DBJ_ATOM_TEST(the_converter(std::deque<char>	{  'H', 'e', 'l', 'l', 'o', '!'}));
	DBJ_ATOM_TEST(the_converter(std::deque<wchar_t>	{ L'H', L'e', L'l', L'l', L'o', L'!'}));
	DBJ_ATOM_TEST(the_converter(std::deque<char16_t>{ u'H', u'e', u'l', u'l', u'o', u'!'}));
	DBJ_ATOM_TEST(the_converter(std::deque<char32_t>{ U'H', U'e', U'l', U'l', U'o', U'!'}));

	// the four standard char types in std::forward_list
	DBJ_ATOM_TEST(the_converter(std::forward_list<char>	{  'H', 'e', 'l', 'l', 'o', '!'}));
	DBJ_ATOM_TEST(the_converter(std::forward_list<wchar_t>	{ L'H', L'e', L'l', L'l', L'o', L'!'}));
	DBJ_ATOM_TEST(the_converter(std::forward_list<char16_t>{ u'H', u'e', u'l', u'l', u'o', u'!'}));
	DBJ_ATOM_TEST(the_converter(std::forward_list<char32_t>{ U'H', U'e', U'l', U'l', U'o', U'!'}));

	// we can serve stunt men to
	DBJ_ATOM_TEST(the_converter(static_cast<const char(&)[]>("\"Abra Ca Dabra\"")));
	DBJ_ATOM_TEST(the_converter(static_cast<const wchar_t(&)[]>(L"\"Abra Ca Dabra\"")));
	DBJ_ATOM_TEST(the_converter(static_cast<const char16_t(&)[]>(u"\"Abra Ca Dabra\"")));
	DBJ_ATOM_TEST(the_converter(static_cast<const char32_t(&)[]>(U"\"Abra Ca Dabra\"")));

	// then string views, and so on ...
	// now lets test non char containers
	// rightly so, none of this compiles
	// DBJ_SHOW(the_converter(std::array<int, 6>{  1,2,3,4,5,6 }));
	// DBJ_SHOW(the_converter(std::vector<int>{  1, 2, 3, 4, 5, 6 }));
	// DBJ_SHOW(the_converter(std::basic_string<int>{  1, 2, 3, 4, 5, 6 }));
}

DBJ_TEST_UNIT(metaconvertortest) {

	test_conversion(dbj::range_to_string    );
	test_conversion(dbj::range_to_wstring   );
	test_conversion(dbj::range_to_u16string );
	test_conversion(dbj::range_to_u32string );
}

DBJ_TEST_SPACE_CLOSE

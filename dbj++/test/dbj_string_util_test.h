#pragma once

// #include "dbj_string_util.h"
#include "../util/dbj_string_compare.h"

DBJ_TEST_SPACE_OPEN(dbj_string_util)

DBJ_TEST_UNIT(dbj_string_util_replace_inplace) {

	using namespace std::literals;

	const  std::string
		new_string
		= DBJ_ATOM_TEST(dbj::str::replace_inplace("abra ka dabra"sv, "ka"sv, "malaga"sv));

	const  std::wstring
		new_wstring
		= DBJ_ATOM_TEST(dbj::str::replace_inplace(L"abra ka dabra"sv, L"ka"sv, L"malaga"sv));

	const  std::u16string
		new_u16string
		= DBJ_ATOM_TEST(dbj::str::replace_inplace(u"abra ka dabra"sv, u"ka"sv, u"malaga"sv));

	const  std::u32string
		new_u32string
		= DBJ_ATOM_TEST(dbj::str::replace_inplace(U"abra ka dabra"sv, U"ka"sv, U"malaga"sv));
}

DBJ_TEST_UNIT(dbj_string_util_remove_first_sub) {

	using namespace std::literals;

	const  std::string
		new_string
		=  DBJ_ATOM_TEST( dbj::str::remove_first_sub("abra ka dabra"sv, "ka"sv) );

	const  std::wstring
		new_wstring
		= DBJ_ATOM_TEST( dbj::str::remove_first_sub(L"abra ka dabra"sv, L"ka"sv) );

	const  std::u16string
		new_u16string
		= DBJ_ATOM_TEST( dbj::str::remove_first_sub(u"abra ka dabra"sv, u"ka"sv) );

	const  std::u32string
		new_u32string
		= DBJ_ATOM_TEST( dbj::str::remove_first_sub(U"abra ka dabra"sv, U"ka"sv) );
}

DBJ_TEST_UNIT(dbj_string_util_remove_all_subs) {

	using namespace std::literals;

	const  std::string
		new_string
		= DBJ_ATOM_TEST(dbj::str::remove_all_subs("abra ka dabra"sv, "ab"sv));

	const  std::wstring
		new_wstring
		= DBJ_ATOM_TEST(dbj::str::remove_all_subs(L"abra ka dabra"sv, L"ab"sv));

	const  std::u16string
		new_u16string
		= DBJ_ATOM_TEST(dbj::str::remove_all_subs(u"abra ka dabra"sv, u"ab"sv));

	const  std::u32string
		new_u32string
		= DBJ_ATOM_TEST(dbj::str::remove_all_subs(U"abra ka dabra"sv, U"ab"sv));
}

DBJ_TEST_UNIT(dbj_string_util_lowerize) {

	using namespace std::literals;

	// no const allowed since  we lowerize in place
	{
		static char specimen[]{ "ABRA KA DABRA" };
		static const auto specimen_size = _countof(specimen);
		DBJ_ATOM_TEST(dbj::str::lowerize(&specimen[0], &specimen[specimen_size - 1]));
	}
	{
		static char specimen[]{ "ABRA KA DABRA" };
		DBJ_ATOM_TEST(dbj::str::lowerize(specimen));
	}
}

DBJ_TEST_UNIT(dbj_string_util_ui_string_compare) {

	using namespace std::literals;

	auto test = []( auto s1, auto s2 ) -> bool  {
		return 
			(0 == dbj::str::ui_string_compare(s1.data(), s2.data(),true ))
			+
			(0 == dbj::str::ui_string_compare(s1.data(), s2.data(),false));
	};

    DBJ_ATOM_TEST(test( "ABRA"sv,  "ABRA"sv));
    DBJ_ATOM_TEST(test(L"AbrA"sv, L"ABRA"sv));
    //auto r3 = DBJ_ATOM_TEST(test(u"ABRA"sv, u"AbrA"sv));
    // auto r4 = DBJ_ATOM_TEST(test(U"abra"sv, U"abra"sv));
}


DBJ_TEST_UNIT(dbj_string_util_ui_is_prefix) {

	using namespace std::literals;

	auto test = [](auto s1, auto s2) -> bool {
		return
			dbj::str::is_view_prefix_to_view(s1, s2);
	};

	DBJ_ATOM_TEST(test("ABRA"sv, "ABRA KA DABRA"sv));
	DBJ_ATOM_TEST(test(L"AbrA"sv, L"ABRA ka dabra"sv));
	DBJ_ATOM_TEST(test(u"ABRA"sv, u"AbrA ka DABRA"sv));
	DBJ_ATOM_TEST(test(U"abra"sv, U"abra ka dabra"sv));
}

DBJ_TEST_UNIT(clasical_string_utils)
{
	using namespace std;
	using util = ::dbj::str_util_char;

	std::string required_state = "ABR A\f \n KA DAB\r RA\t \v";
	std::string  test_input[]{
		"        AAABBBBRRRR AAAA        \f \n KKKKAAAAA         DDDAAAABBB\r RRRRAAAA      \t         \v     "
	};

	util::string_type s1{ test_input[0] };
	auto compressed = util::compressor(s1, "ABRKD ");
	auto normalized = util::normalizer(compressed);
	auto cleaned	= util::remove_chars(normalized);

	::dbj::console::print (
		"\nInput:\n[", util::presentable(s1).c_str(),
		"\nCompressed:\n[%s]", util::presentable(compressed).c_str(),
		"\nNormalized:\n[%s]", util::presentable(normalized).c_str(),
		"\nCleaned:\n[%s]", util::presentable(cleaned).c_str()
	);
}


DBJ_TEST_UNIT(optimal_buffer)
{
	// std::array<char, 10> str;

	DBJ_ATOM_TEST(dbj::str::buffer_optimal_size);

	// returns :std::array<char, dbj::str::buffer_optimal_size >
	auto buf = ::dbj::str::optimal_buffer();

	// returns :std::array<char, dbj::str::buffer_optimal_size >
	auto wbuf = ::dbj::str::optimal_wbuffer();

	[[maybe_unused]]  auto[ptr, erc] = std::to_chars(buf.data(), buf.data() + buf.size(), LONG_MAX);

	DBJ_TEST_ATOM(::dbj::dbj_ordinal_string_compareA(buf.data(), "42", true));

	{
		// string split testing
		std::string required_state = "ABR A\f \n KA DAB\r RA\t \v";
		std::string  test_input(
			"        AAABBBBRRRR AAAA        \f \n KKKKAAAAA         DDDAAAABBB\r RRRRAAAA      \t         \v     "
		);

		DBJ_ATOM_TEST(::dbj::str::fast_string_split(required_state));
		DBJ_ATOM_TEST(::dbj::str::fast_string_split(test_input));
		DBJ_ATOM_TEST(::dbj::str::fast_string_split("Andra\nBabra\tKa Dabra\vKod\fPekara"));
	}
}

DBJ_TEST_SPACE_CLOSE
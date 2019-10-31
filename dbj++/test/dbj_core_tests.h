#pragma once

// #include "../core/dbj++core.h"

DBJ_TEST_SPACE_OPEN(core_tests)

// Test BUFfer
#define DBJ_T_BUF( F,x ) ::dbj::fmt::print("\n\nExpression: %s\n\tResult\n\tType: %s\n\tValue:" F,\
	(#x), typeid(x).name(), (x) )

namespace u = ::dbj::core::util;
using ::dbj::fmt::print;

DBJ_TEST_UNIT(core_format_utils)
{
	std::error_code ec_;

	/*
	BIG NOTE: if you mistake a single formating code probably everything
	on the console and in the app will go *very* wrong, and UCRT
	crash will be spectacular
	*/
	auto rez = dbj::fmt::to_buff("%s", "ABCDEFGH");
	DBJ_VERIFY(!std::strcmp(rez.data(), "ABCDEFGH"));

	auto wrez = dbj::fmt::to_wbuff(L"%s", L"ABCDEFGH");
	DBJ_VERIFY(!std::wcscmp(wrez.data(), L"ABCDEFGH"));

	// uses u::TIME_STAMP_SIMPLE_MASK by default
	DBJ_T_BUF("%s", u::make_time_stamp(ec_, u::TIME_STAMP_SIMPLE_MASK));
	DBJ_T_BUF("%s", u::make_time_stamp(ec_, u::TIME_STAMP_FULL_MASK));
}

DBJ_TEST_UNIT(core_utils)
{
	using namespace ::std::string_view_literals;

	{
		// mid value
		// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0811r2.html
		DBJ_T_BUF( "%3.3f", u::midpoint(3.3f, 7.7f ) );
		DBJ_T_BUF( "%3.3f", u::midpoint(3.3, 7.7 ) );
		DBJ_T_BUF( "%u",	u::midpoint(3U, 7U ) );
		DBJ_T_BUF( "%ul",	u::midpoint(3l, 7l ) );
		// and so on ... numbers only
	}

	std::error_code ec_;
    // env vars
	DBJ_T_BUF("%s", u::dbj_get_envvar("USERNAME", ec_ ));

	// programdata path
	DBJ_T_BUF("%s", u::program_data_path(ec_));
	DBJ_T_BUF("%s", ec_);
}

DBJ_TEST_SPACE_CLOSE

#undef DBJ_T_BUF


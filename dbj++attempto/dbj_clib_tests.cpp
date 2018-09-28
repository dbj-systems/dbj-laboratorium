#include "pch.h"
#include "../dbj++clib/dbjclib.h"
// 'hidden' inside dbj++clib
extern "C" void dbj_string_trim_test();

DBJ_TEST_UNIT(dbj_c_lib)
{
	using auto_char_arr = std::unique_ptr<char>;
	auto_char_arr to_be_freed_1{ dbj::clib::dbj_strdup("Mamma mia!?") };
	auto_char_arr to_be_freed_2{ dbj::clib::dbj_strndup(to_be_freed_1.get(), 5) };
	auto_char_arr to_be_freed_3{
		dbj::clib::dbj_str_shorten
		  ("Abra Ka dabra", " ")
	};
}

DBJ_TEST_UNIT(dbj_string_trim)
{
	dbj_string_trim_test();
}

DBJ_TEST_UNIT(clang_and_dbj)
{
	// reaching to C code
	using namespace DBJ::clib;

	::dbj::clib::error_descriptor *loc_desc_0, *loc_desc_2, *loc_desc_3;
	// begin block
	{
		DBJ::entry_exit{
			[&]() {
			// take one
			loc_desc_0 =
::dbj::clib::dbj_error_provider.create(__LINE__, __FILE__, 0, nullptr);
				},
				[&]() {
					// take two
					loc_desc_2 =
::dbj::clib::dbj_error_provider.create(__LINE__, __FILE__, 0, nullptr);

					// release the 2 but leave the 1
					dbj_error_provider.release(&loc_desc_2);

					// take three, should reuse the second registry slot
					// thus loc_desc_2 == loc_desc_3
					loc_desc_3 =
::dbj::clib::dbj_error_provider.create(__LINE__, __FILE__, 0, nullptr);
					}
		};
	}

	auto ld0 = *loc_desc_0;
	auto ld3 = *loc_desc_3;

	auto DBJ_MAYBE(dummy) = true;
}
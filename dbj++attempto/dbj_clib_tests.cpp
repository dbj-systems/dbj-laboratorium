#include "pch.h"
#define TEST_DBJ_DYNAMIC
#include "../dbj++clib/dbjclib.h"
// #include "../dbj_en_dictionary/dbj_en_dictionary.h"

using namespace ::std::string_view_literals;

DBJ_TEST_UNIT(dbj_string_c_lib)
{
	using namespace dbj::clib;
#ifdef TEST_DBJ_DYNAMIC
	char buf_here[BUFSIZ]{0};
	dbj_string_list_test( & buf_here);
	::dbj::console::prinf("%s", buf_here);
#endif
	dbj_string_test();
}
#undef TEST_DBJ_DYNAMIC

#define DBJ_SLL_TESTING
#include "../dbj++clib/dbj_sll/dbj_sll.h"
DBJ_TEST_UNIT(dbj_c_lib_sll)
{
	auto test = []() {
		for (int k = 0; k < BUFSIZ; k++)
		{
			test_dbj_sll(false);
		}
	};
	dbj::fmt::print("\ndbj clib SLL test finished in %s "sv,
		::dbj::kalends::miliseconds_measure(test)
	);
}
#undef DBJ_SLL_TESTING

DBJ_TEST_UNIT(dbj_c_lib)
{
	auto dbj_strndup_test = []() {
		for (int k = 0; k < BUFSIZ; k++)
		{
			{
				using auto_char_arr = std::unique_ptr<char>;
				auto_char_arr to_be_freed_1{ dbj::clib::dbj_strdup("Mamma mia!?") };
				auto_char_arr to_be_freed_2{ dbj::clib::dbj_strndup(to_be_freed_1.get(), 5) };
				auto_char_arr to_be_freed_3{
					dbj::clib::dbj_str_shorten
					  ("Abra Ka dabra", " ")
				};
			}
		}
	};

	dbj::fmt::print("\ndbj clib dbj strndup test finished in %s"sv,
	::dbj::kalends::miliseconds_measure(dbj_strndup_test)
		) ;
}

// 'hidden' inside dbj++clib
extern "C" void dbj_string_trim_test();
DBJ_TEST_UNIT(dbj_string_trim)
{
	// SEGV currently -- dbj_string_trim_test();
}

#define DBJ_ERR(n) ::dbj::clib::dbj_error_service.create(__LINE__, __FILE__, n, nullptr)

DBJ_TEST_UNIT(dbj_err_system)
{
	// reaching to C code
	using namespace ::dbj::clib;

	auto test = [](unsigned int err_num_) {
		using error_descriptor = ::dbj::clib::error_descriptor;
		auto * err_desc_0 = DBJ_ERR(err_num_);

		_ASSERTE(::dbj::clib::dbj_error_service.is_valid_descriptor(err_desc_0));
		::dbj::clib::dbj_error_service.release(&err_desc_0);
		_ASSERTE(false == ::dbj::clib::dbj_error_service.is_valid_descriptor(err_desc_0));
	};

	test(::dbj::clib::dbj_error_code::DBJ_EC_BAD_ERR_CODE);
	test(::dbj::clib::dbj_error_code::DBJ_EC_BAD_STD_ERR_CODE);
	test(::dbj::clib::dbj_error_code::DBJ_EC_DBJ_LAST_ERROR);
	test(::dbj::clib::dbj_error_code::DBJ_EC_INDEX_TOO_LARGE);
	test(::dbj::clib::dbj_error_code::DBJ_EC_INDEX_TOO_SMALL);
	test(::dbj::clib::dbj_error_code::DBJ_EC_INVALID_ARGUMENT);

	auto DBJ_MAYBE(dummy) = true;
}

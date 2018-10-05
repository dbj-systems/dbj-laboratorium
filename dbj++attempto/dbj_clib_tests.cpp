#include "pch.h"
#include "../dbj++clib/dbjclib.h"
// 'hidden' inside dbj++clib
extern "C" void dbj_string_trim_test();

DBJ_TEST_UNIT(dbj_sll)
{
	dbj::clib::test_dbj_sll();
}

DBJ_TEST_UNIT(dbj_string_c_lib)
{
	dbj::clib::dbj_string_test();
}

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
	// SEGV currently -- dbj_string_trim_test();
}

#if 0
#define DBJ_ERR(n) DBJ::clib::dbj_error_service.create(__LINE__, __FILE__, n, nullptr)

DBJ_TEST_UNIT(dbj_err_system)
{
	// reaching to C code
	using namespace DBJ::clib;

	auto test = []( unsigned int err_num_ ) {
		using error_descriptor = DBJ::clib::error_descriptor ;
		auto * err_desc_0 = DBJ_ERR(err_num_);

		_ASSERTE( DBJ::clib::dbj_error_service.is_valid_descriptor(err_desc_0) );
		DBJ::clib::dbj_error_service.release(&err_desc_0) ;
		_ASSERTE( false == DBJ::clib::dbj_error_service.is_valid_descriptor(err_desc_0) );
	};

	test(::dbj::clib::dbj_error_code::DBJ_EC_BAD_ERR_CODE);
	test(::dbj::clib::dbj_error_code::DBJ_EC_BAD_STD_ERR_CODE);
	test(::dbj::clib::dbj_error_code::DBJ_EC_DBJ_LAST_ERROR);
	test(::dbj::clib::dbj_error_code::DBJ_EC_INDEX_TOO_LARGE);
	test(::dbj::clib::dbj_error_code::DBJ_EC_INDEX_TOO_SMALL);
	test(::dbj::clib::dbj_error_code::DBJ_EC_INVALID_ARGUMENT);

	auto DBJ_MAYBE(dummy) = true;
}

#endif
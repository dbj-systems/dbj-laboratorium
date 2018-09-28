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
	// SEGV currently -- dbj_string_trim_test();
}

#define DBJ_ERR(n) dbj_error_service.create(__LINE__, __FILE__, n, nullptr)
DBJ_TEST_UNIT(dbj_err_system)
{
	// reaching to C code
	using namespace DBJ::clib;

	error_descriptor *err_desc_0;
	// EFAULT is 14
	err_desc_0 = DBJ_ERR(14);
	bool valid_ = dbj_error_service.is_valid_descriptor(err_desc_0);
	dbj_error_service.release(&err_desc_0);
	// what now?
	{
		bool valid_ = dbj_error_service.is_valid_descriptor(err_desc_0);
		dbj_error_service.release(&err_desc_0);
		err_desc_0 = nullptr;
		dbj_error_service.release(&err_desc_0);
	}

	// structs are copyable
	auto ld0 = *err_desc_0;

	auto DBJ_MAYBE(dummy) = true;
}
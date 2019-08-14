#pragma once
#ifdef DBJ_C_LIB_TEST

#define DBJ_STRING_TEST
#include <dbj++clib/dbjclib.h>

namespace dbj_test {

	char specimen[]{"0123456789"};

	extern "C" void basic_str_routines() {

		char * p1 = dbj_strdup(specimen);
		_ASSERTE(0 == strcmp(p1, specimen));
		char * p2 = dbj_strndup(specimen, 4);
		_ASSERTE( 0 == strcmp(p2, "0123") );
		char * p3 = dbj_str_shorten(specimen, p2);
		_ASSERTE(0 == strcmp(p3, "456789"));

		DBJ_MULTI_FREE(p1, p2, p3);
	}

	extern "C" void dbj_string_trim_test();

	extern "C" void dbj_c_lib()
	{
		basic_str_routines();
		dbj_string_trim_test();
		dbj_string_test();
	}
} // dbj_test

#endif
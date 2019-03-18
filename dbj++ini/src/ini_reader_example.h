#pragma once
#ifdef DBJ_INI_READER_TESTING

#include <cstdio>
#include "../dbj++ini.h"

// argv[0] must be the valid ini file full path
int test_dbj_ini_reader( int argc, char * argv[])
{
	using namespace std ;
	using namespace dbj::ini ;

	ini_reader const & reader = ini_reader_instance(argv[0]);

    if (reader.parse_error() < 0) {
        fprintf( stderr, "Can't load 'test.ini'\n");
        return 1;
    }

	wprintf(L"\n\nConfig loaded from 'test.ini': \nversion=%3d\nname=%16S\nemail=%16s\npi=%3.3f\nactive=%6S"
		, reader.get_integer("protocol", "version", -1) 
		, reader.get("user", "name", "UNKNOWN").get()
		, reader.get("user", "email", "UNKNOWN").get()
		, reader.get_real("user", "pi", -1) 
		, (reader.get_bool("user", "active", true) ? "true" : "false")
	);

	wprintf(L"\n\nList in the ini file:\n"
		"\n[list]"
		"\nkey = A"
		"\nkey = B"
		"\nkey = C"
		"\n\nParsed as:\n%S",
		reader.get("list","key", "null").get()
	);

    return 0;
}
#endif // DBJ_INI_READER_TESTING

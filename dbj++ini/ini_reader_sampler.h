#pragma once
#ifdef DBJ_INI_READER_TESTING

#include <cstdio>
#include "dbj++ini.h"

// argv[0] must be the valid ini file full path
int test_dbj_ini_reader( int argc, dbj::chr_buf::yanb argv[])
{
	using namespace std ;
	using namespace dbj::ini ;

	ini_reader const & reader = ini_reader_instance(argv[0].data());

    if (reader.parse_error() < 0) {
        fprintf( stderr, "Can't load %s\n", argv[0].data());
        return 1;
    }

	::dbj::fmt::print
	("\n\nConfig loaded from 'test.ini': \nversion=%3d\nname=%16s\nemail=%16s\npi=%3.3f\nactive=%6s"
		, reader.get_integer("protocol", "version", -1) 
		, reader.get("user", "name", "UNKNOWN")
		, reader.get("user", "email", "UNKNOWN")
		, reader.get_real("user", "pi", -1) 
		, (reader.get_bool("user", "active", true) ? "true" : "false")
	);

	::dbj::fmt::print
		("\n\nList in the ini file:\n"
		"\n[list]"
		"\nkey = A"
		"\nkey = B"
		"\nkey = C"
		"\n\nParsed as:\n%s",
		reader.get("list","key", "null")
	);

    return 0;
}
#endif // DBJ_INI_READER_TESTING

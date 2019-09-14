#pragma once
#define DBJ_INI_READER_TESTING
#ifdef DBJ_INI_READER_TESTING
/* 
code bellow expects this ini file
-----------------------------------------------------------

; Test config file 

[protocol]             ; Protocol configuration
version=6              ; IPv6

[user]
name = Bob Smith       ; Spaces around '=' are stripped
email = bob@smith.com  ; And comments (like this) ignored
active = true          ; Test a boolean
pi = 3.14159           ; Test a floating point number

[list]
key=A
key=B
key=C
*/

#include <cstdio>
#include "dbj++ini.h"

namespace ini_reader_sampler {

	using buffer		= typename dbj::ini::buffer;
	using buffer_type	= typename dbj::ini::buffer_type ;

	// argv[0] must be the valid ini file full path
	int test_dbj_ini_reader( buffer_type argv)
	{
		using namespace std;
		using namespace dbj::ini;

		ini_reader const& reader = ini_reader_instance(argv.data());

		if (reader.parse_error() < 0) {
			DBJ_PRINT("Can't load %s\n", argv.data());
			return 1;
		}

		DBJ_PRINT
		("\n\nConfig loaded from 'test.ini': \nversion=%3d\nname=%16s\nemail=%16s\npi=%3.3f\nactive=%6s"
			, reader.get_integer("protocol", "version", -1)
			, reader.get("user", "name", "UNKNOWN")
			, reader.get("user", "email", "UNKNOWN")
			, reader.get_real("user", "pi", -1)
			, (reader.get_bool("user", "active", true) ? "true" : "false")
		);

		DBJ_PRINT
		("\n\nList in the ini file:\n"
			"\n[list]"
			"\nkey = A"
			"\nkey = B"
			"\nkey = C"
			"\n\nParsed as:\n%s",
			reader.get("list", "key", "null")
		);

		return 0;
	}
} // namespace ini_reader_sampler 

#endif // DBJ_INI_READER_TESTING

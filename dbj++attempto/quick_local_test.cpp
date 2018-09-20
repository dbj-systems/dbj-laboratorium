#include "pch.h"
#include "../dbj++clib/dbjclib.h"

using namespace ::std ;

// must be limited string
// that is with EOS ('\0') a the end
inline std::string trimmer(string_view text)
{
	char * p1 = 0;
	char * p2 = 0;
	DBJ::clib::dbj_string_trim(text.data(), &p1, &p2);
	return { p1, p2 };
}

DBJ_TEST_UNIT(dbj_string_trim)
{
	using namespace string_view_literals;

	auto target = "LINE O FF\n\rTE\v\tT"sv;
	constexpr std::string_view text[]{
		{ "   LINE O FF\n\rTE\v\tT    "sv },
		{ "   LINE O FF\n\rTE\v\tT"sv },
		{    "LINE O FF\n\rTE\v\tT"sv },
		{ "     "sv }
	};

	_ASSERTE(target == trimmer(text[0]));
	_ASSERTE(target == trimmer(text[1]));
	_ASSERTE(target == trimmer(text[2]));
	// on trim, spaces are collapsing 
	// to empty string
	_ASSERTE("" == trimmer(text[3]));

	// To "trim" the string actually means to shorten it 
	// by inserting 0 aka "end of string" where required
	// which in C/C++ is better not to be done
	// this is because of 
	// R/O memory
	//const char * ro = "READONLY";
	//char * bang_ = (char *) & ro[0] ;
	// write access violation
	// bang_[0] = '!';
}

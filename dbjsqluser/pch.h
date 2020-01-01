/*
(c) 2019 by dbj@dbj.org -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/
*/  
#ifndef _PCH_INC_
#define _PCH_INC_


// we reepeat this possibly on few other headers
// to cut down on dependancies

#ifndef __cplusplus
#error DBJ NANOLIB requires C++ compiler
#endif

// C++ language version detection (C++20 is speculative):
// Note: VC14.0/1900 (VS2015) lacks too much from C++14.

#ifndef DBJ_CPLUSPLUS

#if defined(_MSVC_LANG) && !defined(__clang__)
#define DBJ_CPLUSPLUS (_MSC_VER == 1900 ? 201103L : _MSVC_LANG)
#else
#define DBJ_CPLUSPLUS __cplusplus
#endif

#define DBJ_CPP98_OR_GREATER (DBJ_CPLUSPLUS >= 199711L)
#define DBJ_CPP11_OR_GREATER (DBJ_CPLUSPLUS >= 201103L)
#define DBJ_CPP14_OR_GREATER (DBJ_CPLUSPLUS >= 201402L)
#define DBJ_CPP17_OR_GREATER (DBJ_CPLUSPLUS >= 201703L)
#define DBJ_CPP20_OR_GREATER (DBJ_CPLUSPLUS >= 202000L)

#if (DBJ_CPLUSPLUS < 201703L)
#error DBJ SQLITE requires the standard C++17 compiler
#endif
#if (DBJ_CPLUSPLUS > 201703L)
#error DBJ SQLITE  is not ready yet for the standard C++20 (or higher)
#endif

#endif //  DBJ_CPLUSPLUS

#include <future>
#include <dbj--sqlite/dbj++sql.h>
// includes dbj_nanolib also
#include "dbj--nanolib/dbj++tu.h"




#endif // !_PCH_INC_

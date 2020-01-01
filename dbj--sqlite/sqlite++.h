#pragma once
#ifndef DBJ_SQLITE_INC
#define DBJ_SQLITE_INC

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

#ifndef _WIN64
#error SQLITE lib and dll, we use, are built for x64 applications.
#endif


/*
dbj.org -- 2019-01-26 -- created

wrap the sqlite into the namespace

obviously does *not* hide the #define's 
for example SQLITE_OK
so please learn what are they 
in sqlite3 documentation

2018-11-03	dbj@dbj.org
*/
#ifdef __cplusplus
namespace sqlite{
#endif

#include "sqlite3/sqlite3.h"

#ifdef __cplusplus
}
#endif


/*
final app depends on sqlite3.dll (x64) being available
*/
#endif // !DBJ_SQLITE_INC

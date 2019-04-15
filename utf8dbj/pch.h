#pragma once
#ifndef PCH_H
#define PCH_H

#define THIS_APP_TITLE L"dbj++utf8 (c) by dbj.systems (" __DATE__ ")"

#define NOMINMAX
#define STRICT
#include <windows.h>
#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <assert.h>
#include <math.h>
#include <fcntl.h>  
#include <io.h> 
#include <stdio.h> 
#include <wchar.h> 
#ifndef _HAS_CXX17
#	error C++17 please ...
#endif
#	define UNREF(...) (void)noexcept(__VA_ARGS__)
#define ST_2(x) #x
#define ST_1(x) ST_2(x)
#define ST(x) ST_1(x)
#define TU(x) printf("\n",ST(x));

//// first a good decades old VERIFY macro
//namespace dbj {
//	[[noreturn]]
//	inline void terror
//	(const char * msg_, const char * file_, const int line_)
//	{
//		assert(msg_);
//		assert(file_);
//		assert(line_);
//		::fwprintf(stderr, L"\n\ndbj++ ERROR:%S\n%S (%d)", msg_, file_, line_);
//		exit(EXIT_FAILURE);
//	}
//
//#define DBJ_VERIFY_(x, file, line ) if (false == (x) ) ::dbj::terror( #x ", failed", file, line )
//#define DBJ_VERIFY(x) DBJ_VERIFY_(x,__FILE__,__LINE__)
//} // dbj

#endif //PCH_H

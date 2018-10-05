/*
Copyright 2018 by dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#ifdef __clang__
/*
http://clang.llvm.org/docs/UsersManual.html#controlling-diagnostics-in-system-headers
*/
#pragma clang system_header
#endif /* __clang__ */

#if defined (_MSC_VER)
#define PLATFORM "Windows"
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
// we do the above since we use MSVC UCRT *from* the clang c code
#elif defined (__linux)
#define PLATFORM "Linux"
#endif


#if defined( __clang__ ) && ! defined( __cplusplus )
# if !defined(__STDC_VERSION__) ||  (__STDC_VERSION__ < 199901L)
#error    Your compiler is not conforming to C99
#error    this requires the macro __STDC_VERSION__ to be set to the
#error    indicated value (or larger).
#error    NOTE: For C11, __STDC_VERSION__ == 201112L
#endif

#define FREE(p) do{  if (p) free((void *)p); p = 0;}while(0)

#ifdef _MSC_VER
#include <crtdbg.h>
#define DBJ_ASSERT _ASSERTE
#else
#include <assert.h>
#define DBJ_ASSERT assert
#endif

/* use this to remove unused code */
/* this verion does not evaluate the expression */
// #define DBJ_REMOVE(expr) typedef char __static_assert_t[sizeof(expr) != 0]
#define DBJ_REMOVE(expr)

#	if ! defined(_MSC_EXTENSIONS)
#error Need MSC EXTENSIONS DEFINED
#	endif

#if !defined( _WIN32 ) && !defined(_WIN64)
#error Need _WIN32 or _WIN64
#endif

#endif
/*
Note: while inside c++ all is in the dbj::clib namespace
*/
#ifdef __cplusplus
namespace dbj { namespace clib {
	extern "C" {
#endif

typedef unsigned char	uchar_t;

#ifdef _WIN64
		typedef unsigned __int64 size_t;
#else
		typedef unsigned int	size_t;
#endif

/*
strdup and strndup are defined in POSIX compliant systems as :
char *strdup(const char *str);
char *strndup(const char *str, size_t len);
*/
		char * dbj_strdup(const char *s);
/*
The strndup() function copies at most len characters from the string str
always null terminating the copied string.
*/
		char * dbj_strndup(const char *s, size_t n);
// remove chars given in the string arg
// return the new string
// user has to free() eventually
// both arguments must be zero limited
		char * dbj_str_shorten(const char * str_, const char * chars_to_remove_);

#include "dbj_error_codes.h"
#include "dbj_error.h"
#include "dbj_trim.h"
#include "dbj_string.h"
#include "dbj_sll.h"

#ifdef __cplusplus
	} // extern "C"
} // eof namespace clib 
} // eof namespace dbj 
#endif


/*
Copyright 2017,2018 by dbj@dbj.org

Licensed under the GNU GPL License, Version 3.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the file LICENSE enclosed in
this project.

https://www.gnu.org/licenses/gpl-3.0.html

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#ifndef DBJ_CLIB_PRESENT
#define DBJ_CLIB_PRESENT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>

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
#define DBJ_REMOVE(expr) (void)(sizeof(expr))

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

#ifndef size_t
#ifdef _WIN64
		typedef unsigned __int64 size_t;
#else
		typedef unsigned int	size_t;
#endif
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

#include "./dbj_error/dbj_error_codes.h"
#include "./dbj_error/dbj_error.h"
#include "./dbj_string/dbj_trim.h"
#include "./dbj_string/dbj_string.h"
// #include "./dbj_sll/dbj_sll.h" 
#include "./dbj_string_list/dbj_string_list.h"

		extern const unsigned long dbj_random_kiss();

#ifdef __cplusplus
	} // extern "C"
} // eof namespace clib 
} // eof namespace dbj 
#endif

#endif // ! DBJ_CLIB_PRESENT

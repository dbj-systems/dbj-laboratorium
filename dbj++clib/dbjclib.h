/*
Copyright 2017 by dbj@dbj.org

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

#if defined( __clang__ ) && ! defined( __cplusplus )

# if !defined(__STDC_VERSION__) ||  (__STDC_VERSION__ < 199901L)
/* Your compiler is not conforming to C99, since
   this requires the macro __STDC_VERSION__ to be set to the
   indicated value (or larger).

   NOTE: For C11, __STDC_VERSION__ == 201112L
*/
#define DBJ_EVAL_REMOVE(expr) typedef char __static_assert_t[(expr) != 0]
#define DBJ_REMOVE(expr) typedef char __static_assert_t[sizeof(expr) != 0]
#else
#define DBJ_EVAL_REMOVE(expr) typedef char __static_assert_t[(expr) != 0]
#define DBJ_REMOVE(expr) typedef char __static_assert_t[sizeof(expr) != 0]
# endif
/*
#	if ! defined(_WCHAR_T_DEFINED)
#error	Need _WCHAR_T_DEFINED
#	endif
*/

#	if ! defined(_MSC_EXTENSIONS)
#error Need MSC EXTENSIONS DEFINED
#	endif


#if !defined( _WIN32 ) && !defined(_WIN64)
#error Need _WIN32 or _WIN64
#endif

#endif

#ifdef __cplusplus
namespace dbj::clib {
	extern "C" {
#endif
		typedef struct location_descriptor location_descriptor;

        #define location_descriptor_file_name_size 1024U

		typedef struct location_descriptor {
			unsigned int cache_index;
			unsigned int line;
			char file[location_descriptor_file_name_size];
		} location_descriptor;

		typedef struct LOCATION LOCATION;

		typedef struct LOCATION {

			location_descriptor * 
				(*create)
				(const int line_, const char * file_);

			location_descriptor *
				(*release)
				(location_descriptor **);

	} LOCATION;

		extern LOCATION location_;

#ifdef __cplusplus
	}
} // eof namespace dbj::clib 
#endif


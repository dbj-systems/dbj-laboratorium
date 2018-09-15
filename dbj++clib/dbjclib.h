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
extern "C" {
#endif

	typedef struct location_descriptor location_descriptor;

	#define location_descriptor_file_name_size 1024U

	typedef struct location_descriptor {
		unsigned int cache_index;
		unsigned int line;
		char file[location_descriptor_file_name_size];
	} location_descriptor;

	// return null on cache overflow
	struct location_descriptor *
		create_location_descriptor(const int line_, const char * file_);

	// always return null
	location_descriptor * release_location_descriptor( location_descriptor ** );

#ifdef __cplusplus
}
#endif

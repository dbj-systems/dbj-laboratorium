#pragma once

/*
there ary many ways better than this one
to include/exclude tests at compile time
or even run time...but ... this works
*/

// dbj++ tests
// are not included in dbj++.h
// for using this include it after dbj++.h

#define dbj_ap_env_test   
#define dbj_commander_test   
// #define dbj_console_test   
#define dbj_crt_testing 
#define dbj_core_tests
#define dbj_defval_testing   
#define dbj_kv_store_test   
#define dbj_array_testing   
#define dbj_traits_testing   
#define dbj_win32_tests   
#define dbj_kalends_test   
#define dbj_string_util_test   
#define dbj_string_tests   
#define dbj_any_optional_tests
#define dbj_error_concept_tests
#define dbj_numeric_tests

#ifdef dbj_numeric_tests
#define DBJ_FIBONACCI_TESTING
#include "test/dbj_static_matrix_test.h"
#endif // dbj_numeric_tests


#ifdef dbj_core_tests   
#define _DBJ_META_CONVERTER_TESTING
#include "test\dbj_core_tests.h"
#endif

#ifdef dbj_ap_env_test   
#include "test\dbj_ap_env_test.h"  
#endif
#ifdef dbj_commander_test   
#include "test\dbj_commander_test.h"  
#endif
#ifdef dbj_console_test   
#include "test\dbj_console_test.h"  
#endif
#ifdef dbj_crt_testing   
#include "test\dbj_crt_testing.h"  
#endif
#ifdef dbj_defval_testing   
#include "test\dbj_defval_testing.h"  
#endif
#ifdef dbj_kv_store_test   
#include "test\dbj_kv_store_test.h"  
#endif
#ifdef dbj_array_testing   
#include "test\dbj_array_testing.h"  
#endif
#ifdef dbj_traits_testing   
#include "test\dbj_traits_testing.h"  
#endif
#ifdef dbj_win32_tests   
#include "test\dbj_win32_tests.h"  
#endif
#ifdef dbj_kalends_test   
#include "test\dbj_kalends_test.h"  
#endif

#ifdef dbj_string_util_test   
#include "test\dbj_string_util_test.h"  
#endif
#ifdef dbj_string_tests   
#include "test\dbj_string_tests.h"  
#endif


#ifdef dbj_any_optional_tests   
#include "test\dbj_any_optional_tests.h"
#endif

/* inclusion of this file defines the kind of a licence used */
#include "dbj_license.h"

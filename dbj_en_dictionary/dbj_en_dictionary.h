#pragma once
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>

#ifdef __clang__
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

#define FREE(p) do{  if (p) free((void *)p); p = 0; }while(0)

#ifndef DBJ_ASSERT
#ifdef _MSC_VER
#include <crtdbg.h>
#define DBJ_ASSERT _ASSERTE
#else
#include <assert.h>
#define DBJ_ASSERT assert
#endif
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

/*
Note: while inside c++ all is in the dbj::clib namespace
*/
#ifdef __cplusplus
namespace dbj {
	namespace clib {
		extern "C" {
#endif

			typedef unsigned char	uchar_t;

#ifdef _WIN64
			typedef unsigned __int64 size_t;
#else
			typedef unsigned int	size_t;
#endif

typedef struct dbj_en_dictionary_ dbj_en_dictionary_type ;

// dictionary file has to be in the same folder as the executable
#define dbj_en_dictionary_file "en_dictionary_alpha_words.txt"
// there are no words longer than 1024
#define dbj_en_dictionary_BUFISZ 1024U

#define  dbj_en_dictionary_OK	1
#define  dbj_en_dictionary_ERR	2
#define  dbj_en_dictionary_EOF	4

typedef struct dbj_en_dictionary_retval_ { 
	int status;
	const char * message;
} dbj_en_dictionary_retval ;

typedef char * dbj_en_dictionary_callback_arg_type[];
typedef void   dbj_en_dictionary_callback_retval_type ;

typedef dbj_en_dictionary_callback_retval_type
				(*dbj_en_dictionary_callback) 
					(dbj_en_dictionary_callback_arg_type) ;

// error handling: 
// on error
// retval is one of dbj_en_dictionary_status
// callback arg is populated with error message 

struct dbj_en_dictionary_ 
{
	// arg is the word to be found
	// querying by prefix is allowed
	// arg is populated with all the words found
	// 
	//  char * args[] = {"al"};
	//  dbj_en_dictionary_service.find( args ) ;
	//  
	// minimum length of the prefix is 2!
	// maximum length of the prefix is 64!
	//
	dbj_en_dictionary_retval(*find)(const char *, dbj_en_dictionary_callback);
	// reopen the file
    // callback is called with error message or empty string
	dbj_en_dictionary_retval(*reset)(dbj_en_dictionary_callback);
	// callback 
	// arg is populated with next word read from file
	// on file end, dbj_en_dictionary_status.EOF is returned
	// callback is called with NULL
	dbj_en_dictionary_retval(*next)(dbj_en_dictionary_callback);
	 // callback is called with error message or empty string
	dbj_en_dictionary_retval(*close)(dbj_en_dictionary_callback);
} ;

extern dbj_en_dictionary_type dbj_en_dictionary_service ;

#ifdef __cplusplus
		} // extern "C"
	} // eof namespace clib 
} // eof namespace dbj 
#endif


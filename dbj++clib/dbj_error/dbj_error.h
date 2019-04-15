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

#include <stdbool.h>

#pragma region dbj error
#define error_descriptor_buffer_size 1024U

typedef struct error_descriptor error_descriptor;
/*

dbj_error_service is integrated with C standard error handling
as defined in errno.h

https://www.gnu.org/software/libc/manual/html_node/Error-Reporting.html#Error-Reporting

error_descriptor.error_code might be one of errno values as
enumerated in <errno.h>

in which case error_descriptor.error_message is obtained by using
the standard C strerror() function 
https://en.cppreference.com/w/c/string/byte/strerror

*/
typedef struct error_descriptor {
	unsigned int runtime_index;
	unsigned int line;
	char file[error_descriptor_buffer_size];
	unsigned int error_code;
	char error_message[error_descriptor_buffer_size];
} error_descriptor;

typedef struct dbj_error dbj_error;

typedef struct dbj_error {

	error_descriptor *
		(*create)
		(const int line_, const char * file_, const int code_, const char * message_ );

	void
		(*release)
		(error_descriptor **);

	error_descriptor *
		(*find)
		(const int runtime_index_ );

	bool
	(*is_valid_descriptor)
		(error_descriptor *);

} dbj_error;

extern dbj_error dbj_error_service;

typedef int dbj_err_handle_type;
#ifdef _MSC_VER
#define DBJ_NOT_ERR -1
#else
static const dbj_err_handle_type DBJ_NOT_ERR = -1;
#endif
extern dbj_err_handle_type dbj_last_error;

/*

dbj++clib error mechanism

The API
-------

if there was no error dbj++clib functions can set the dbj_last_error
to the DBJ_NOT_ERR value.

dbj++clib functions will set dbj_last_error to the  error_descriptor runtime_index
of the error_descriptor created on some error
clients can use the error_descriptor created
by obtaining it using 2 methods:

// does not release the error_descriptor returned 
dbj_error_service.hold ( runtime_index ) 
or

// does return *released* error_descriptor
dbj_error_service.fetch( )

dbj_error_service has internal cache of error_descriptor's

users are responsible to release the descriptors they hold
by using :
dbj_error_service.release( ** error_descriptor ) ;

error_descriptor * dbj_error_service.status( )
this returns the speicial error_descriptor to indicate the 
number of taken slots and the max number of slots
in the cache

int dbj_error_service.reset( )
Completely resets dbj_error_service cache

If there are no more free slots in its internal cache
dbj_error_service reuses the slots starting again from 0.

dbj_error_service is integrated with C standard error handling 
as defined in errno.h
*/

#pragma endregion 


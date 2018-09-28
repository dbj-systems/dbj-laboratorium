#pragma once

#pragma region location descriptor
#define error_descriptor_buffer_size 1024U

typedef struct error_descriptor error_descriptor;

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

	error_descriptor *
		(*release)
		(error_descriptor **);

	error_descriptor *
		(*find)
		(const int runtime_index_ );


} dbj_error;

extern dbj_error dbj_error_provider;

typedef int dbj_err_handle_type;
static const dbj_err_handle_type DBJ_NOT_ERR = -1;
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
dbj_error_provider.hold ( runtime_index ) 
or

// does return *released* error_descriptor
dbj_error_provider.fetch( )

dbj_error_provider has internal cache of error_descriptor's

users are responsible to release the descriptors they hold
by using :
dbj_error_provider.release( ** error_descriptor ) ;

error_descriptor * dbj_error_provider.status( )
this returns the speicial error_descriptor to indicate the 
number of taken slots and the max number of slots
in the cache

int dbj_error_provider.reset( )
Completely resets dbj_error_provider cache

If there are no more free slots in its internal cache
dbj_error_provider reuses the slots starting again from 0.


*/

#pragma endregion 


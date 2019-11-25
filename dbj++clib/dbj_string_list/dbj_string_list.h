#pragma once
/*
(c) 2019 by dbj@dbj.prg -- Licence: GPLv3

manage the 'char **' structure
it is said to be "self contained" because of the sentinel
at the end, it does not need a structure above it,
to keep it's size for example.

  empty:

  char  ** --------->[ char * ] ----------> SENTINEL

  populated:

  char  ** --------->[ char * ] ----------> "ABRA"
					 [ char * ] ----------> "KA"
					 [ char * ] ----------> "DABRA"
					 [ char * ] ----------> SENTINEL
  size == 3
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifndef DBJ_MALLOC
	#define DBJ_MALLOC(N) calloc(1,N)
#endif

#ifdef __cplucplus
extern "C" {
#endif

	typedef char ** dbj_string_list_type;

	/*
	returns the empty list
	*/
	dbj_string_list_type dbj_string_list_new();

	dbj_string_list_type dbj_string_list_append(dbj_string_list_type, const char *);

	/*
	reset the list and return the head
	*/
	dbj_string_list_type dbj_string_list_reset(dbj_string_list_type);

	/*
	head must be freed manually when no longer needed
	*/
	void dbj_string_list_free(dbj_string_list_type);

// #define TEST_DBJ_DYNAMIC
#ifdef TEST_DBJ_DYNAMIC
	void dbj_string_list_test(char(*)[BUFSIZ]);
#endif

#ifdef __cplucplus
}
#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	static char const * DBJ_LOG_FILE_FOLDER = "dbj\\dbj++";
	static char const * DBJ_LOG_FILE_NAME = "dbj++.log";

	/* prints the error and exits the app */
	void dbj_terror(char const * msg_, char const * file_, const unsigned line_);

	static char const * DBJ_TIME_STAMP_FULL_MASK
		= "%Y-%m-%d %H:%M:%S";

	static char const * DBJ_TIME_STAMP_SIMPLE_MASK
		= "%H:%M:%S";

	/*
	copy or use the result immediately
	do not free the result

	id result is null, errno will contain the POSIX err code

	if timestamp_mask_ is NULL, TIME_STAMP_SIMPLE_MASK will be used
	*/
	char * dbj_time_stamp(char const * timestamp_mask_);

	// set the locked_ to TRUE for internal locking
	void dbj_write_to_local_log(char * const, int locked_);

	const char * dbj_get_envvar(char * varname_);
	
#ifdef __cplusplus
}	/*extern "C" */
#endif

// decades old VERIFY macro
#define DBJ_VERIFY_(x, file, line ) if (false == (x) ) dbj_terror( "  " #x " , failed at", file, line )
#define DBJ_VERIFY(x) DBJ_VERIFY_(x,__FILE__,__LINE__)
#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

	static char const * DBJ_LOG_FILE_FOLDER = "dbj\\dbj++";
	static char const * DBJ_LOG_FILE_NAME = "dbj++.log";

	/*
	Write to the local log file
	*/
	void dbj_local_log_file_write(const char * );

	/* prints the error and exits the app */
	void dbj_terror(char const * /*msg_*/, char const * /*file_*/, const unsigned /*line_*/);

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
	char * dbj_time_stamp(char const * /*timestamp_mask_*/);

	/*
	old fashioned timestamp as required by RFC3164
	is without milli seconds
	*/
	void dbj_timestamp_rfc3164(char(* /*timestamp_rfc3164*/ )[0xFF], int /*require_milli_secs*/);


	/*
	format the message by the rules of RFC3164
	and then write to the local log file
	*/
	void dbj_write_to_local_log(
		char * /* priority_name_ */,
		char * /* timestamp_rfc3164*/,
		char * /* local_hostname*/, 
		char * /* syslog_ident*/, 
		char * /* syslog_procid_str*/,
		int    /* lock_for_mt*/,
		const char * /* fmt_string*/,
		va_list /* the message */);
	

	const char * dbj_get_envvar(char * /*varname_*/);

#ifdef __cplusplus
}	/*extern "C" */
#endif

// decades old VERIFY macro
#define DBJ_VERIFY_(x, file, line ) if (false == (x) ) dbj_terror( "  " #x " , failed at", file, line )
#define DBJ_VERIFY(x) DBJ_VERIFY_(x,__FILE__,__LINE__)
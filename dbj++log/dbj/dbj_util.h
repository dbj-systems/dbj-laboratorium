#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

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
		const char * /* the message */ );

#ifdef __cplusplus
}	/*extern "C" */
#endif

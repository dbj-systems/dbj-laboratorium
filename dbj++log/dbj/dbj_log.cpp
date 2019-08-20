#define DBJ_INCLUDE_STD_
#include <dbj++/dbj++required.h>

// #define DBJ_WMAIN_USED <-- since we are building a library we do not need wmain()
#include <dbj++/dbj++.h>

#include "dbj_util.h"
#include "dbj_local_log.h"

using lock_unlock = dbj::sync::lock_unlock;

// #define DBJ_VERIFY(x) _ASSERT_AND_INVOKE_WATSON(x)

void dbj_write_to_local_log(
	char * priority_name_ ,
	char * timestamp_rfc3164,
	char * local_hostname,
	char * syslog_ident,
	char * syslog_procid_str,
	const char * the_message )
{
	lock_unlock locker_;

	DBJ_VERIFY(priority_name_ && timestamp_rfc3164 && local_hostname 
		&& syslog_ident && syslog_procid_str && the_message);

	char datagramm[BUFSIZ * 2] = { 0 }; // yes we hardcode the datagramm size to 1KB
	static const unsigned datagramm_size = sizeof(datagramm);
	   
	DBJ_VERIFY( 1 < sprintf_s(datagramm, datagramm_size,
		"\n%8s |%20s |%16s |%16s%s | %s",
		priority_name_, timestamp_rfc3164 ,
		local_hostname, syslog_ident, syslog_procid_str, the_message));

	::dbj::log::async_log_write<char>(datagramm);
	// above requires write buffer and mechanism that will empty the buffer on exit
}





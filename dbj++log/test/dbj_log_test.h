#include <string.h>
#include <dbj++/core/dbj_crt.h>
#include "../dbj++log.h"

inline char * basename( char * full_path ) {
	char *p = full_path, *pp = 0;
	while ((p = strchr(p + 1, '\\'))) {
		pp = p;
	}
	return (pp ? pp + 1 : p);
}

extern "C" inline int dbj_log_test( int argc, char * module_basename_ )
{
	using namespace ::dbj::log;

// targeting the local syslog server, if one is found
	syslog_init();

// carefull! max len of the tag is 128
// some syslog servers will move the tag to the message if tag len > 128
// some will truncate it to len 128
// tag as app base name, also log to local log file, source is "user"
	syslog_open(module_basename_, syslog_open_options::log_perror );

// openlog(NULL, LOG_PERROR );
// openlog(argv[0], LOG_PERROR , LOG_USER );
//
// above is optional, it is required when source tag needs to be user defined 
// and/or when local logging is required
// and/or when another "facility" (aka source) needs to be defined
// default source is "user"
//
// "%programdata%/dbj/dbj++" ;
// "%programdata%/dbj/dbj++/dbj++.log";

	syslog_emergency("%s", DBJ_ERR_PROMPT("Emergency!"));
	syslog_alert("%s", DBJ_ERR_PROMPT("Alert!"));
	syslog_critical("%s", DBJ_ERR_PROMPT("Critical Event!"));
	syslog_error("%s", DBJ_ERR_PROMPT("Just an Error"));
	syslog_warning("%s", DBJ_ERR_PROMPT("Warning, this is usefull"));
	syslog_notice("%s", DBJ_ERR_PROMPT("You are notified"));
	syslog_info("%s", DBJ_ERR_PROMPT("This is information"));
	syslog_debug("%s", DBJ_ERR_PROMPT("Debug message") );

	return 0;
}
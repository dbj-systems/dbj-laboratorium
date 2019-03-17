#include <string.h>

inline char * basename( char * full_path ) {
	char *p = full_path, *pp = 0;
	while ((p = strchr(p + 1, '\\'))) {
		pp = p;
	}
	return (pp ? pp + 1 : p);
}

extern "C" inline int dbj_log_test( int argc, char * argv[] ) 
{
	using namespace ::dbj::log;

// DBJ TODO : 
// must know the server IP address
// where is it coming from? the ini file? app manifest?
	// syslog_init("192.168.0.202");
	syslog_init();

// carefull! max len of the tag is 128
// some syslog servers will move the tag to the message if tag len > 128
// some will truncate it to len 128
// tag as app base name, also log to local log file, source is "user"
	syslog_open(basename(argv[0]), syslog_open_options::log_perror );

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

	syslog_emergency("%s", "LOG_EMERG"	);
	syslog_alert("%s", "LOG_ALERT"	);
	syslog_critical("%s", "LOG_CRIT"	);
	syslog_error("%s", "LOG_ERR"		);
	syslog_warning("%s", "LOG_WARNING" );
	syslog_notice("%s", "LOG_NOTICE"	);
	syslog_info("%s", "LOG_INFO"	);
	syslog_debug("%s", argv[0]		);

	return 0;
}
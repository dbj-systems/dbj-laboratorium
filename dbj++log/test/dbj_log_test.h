#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SYSLOG_NAMES
#include "../syslog/syslog.h"
#include <string.h>

inline char * basename( char * full_path ) {
	char *p = full_path, *pp = 0;
	while ((p = strchr(p + 1, '\\'))) {
		pp = p;
	}
	return (pp ? pp + 1 : p);
}

extern "C" inline int dbj_log_test( int argc, char * argv[] ) {

// DBJ TODO : 
// must know the server IP address
// where is it coming from? the ini file? app manifest?
init_syslog("192.168.0.202");

// carefull! max len of the tag is 128
// some syslog servers will move the tag to the message if tag len > 128
// some will truncate it to len 128
// tag as app base name, also log to local log file, source is "user"
openlog(basename(argv[0]), LOG_PERROR, NULL );

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

	syslog(LOG_EMERG,	"%s", "LOG_EMERG"	);
	syslog(LOG_ALERT,	"%s", "LOG_ALERT"	);
	syslog(LOG_CRIT,	"%s", "LOG_CRIT"	);
	syslog(LOG_ERR,		"%s", "LOG_ERR"		);
	syslog(LOG_WARNING, "%s", "LOG_WARNING" );
	syslog(LOG_NOTICE,	"%s", "LOG_NOTICE"	);
	syslog(LOG_INFO,	"%s", "LOG_INFO"	);
	syslog(LOG_DEBUG,	"%s", argv[0]		);

	closelog();

	return 0;
}
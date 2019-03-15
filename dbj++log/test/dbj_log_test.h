#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SYSLOG_NAMES
#include "../syslog/syslog.h"

extern "C" inline int dbj_log_test( int argc, char * argv[] ) {

	// DBJ TODO : 
	// must know the server IP address
	// where is it coming from? the ini file? app manifest?
	init_syslog("192.168.0.202");

//	openlog("dbj_TAG", LOG_CONS , LOG_USER );
	
	openlog(NULL, LOG_PERROR | LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);

	// openlog(argv[0], LOG_PERROR , LOG_USER );

	syslog(LOG_EMERG,	"%s", "LOG_EMERG"	);
	syslog(LOG_ALERT,	"%s", "LOG_ALERT"	);
	syslog(LOG_CRIT,	"%s", "LOG_CRIT"	);
	syslog(LOG_ERR,		"%s", "LOG_ERR"		);
	syslog(LOG_WARNING, "%s", "LOG_WARNING" );
	syslog(LOG_NOTICE,	"%s", "LOG_NOTICE"	);
	syslog(LOG_INFO,	"%s", "LOG_INFO"	);
	syslog(LOG_DEBUG,	"%s", argv[0]	);

	closelog();
}
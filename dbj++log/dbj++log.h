#pragma once
/*
(c) 2019 by dbj.systems, author: dbj@dbj.org, Licence GPLv3

syslog + local file log 
-----------------------
+ stderr redirection to the same file

Both async and both following the syslog protocol
Currently RFC3164 is foollowed. Plas is to develop a support for the newer one: RFC5424.

syslog project started from is Copyright (c) 2008 Secure Endpoints Inc.

WARNING: syslog() is UNIX and is blisfully unaware of wchar_t

#include "syslog/syslog.h"
is not done here, thus we have
full decoupling from the syslog(), C interface and most importantly 
from hash defines in the syslog.h
*/

namespace dbj::log {

		void syslog_init(const char * = nullptr /*syslog_server_ip_and_port*/);

		/*
		 Option flag for syslog_open() is this or NULL.
		 if set then local log file will be written to with the same line 
		 as it was sent to the syslog server

		 Default local log file is:
		 "%ProgramData%\\dbj\\dbj++\\dbj++.log"
		 */
		enum class syslog_open_options {
			_null_,
			log_perror = 0x20
		};

		/* 
		facility codes, syslog_open third argument or if NULL 
		LOG_USER aka "user" is the default one
		*/
		enum class syslog_open_facilities {
			_null_,
			log_kern = (0 << 3), 	/* kernel messages */
			log_user = (1 << 3), 	/* random user-level messages */
			log_mail = (2 << 3), 	/* mail system */
			log_daemon = (3 << 3), 	/* system daemons */
			log_auth = (4 << 3), 	/* security/authorization messages */
			log_syslog = (5 << 3), 	/* messages generated internally by syslogd */
			log_lpr = (6 << 3), 	/* line printer subsystem */
			log_news = (7 << 3), 	/* network news subsystem */
			log_uucp = (8 << 3), 	/* UUCP subsystem */
			log_cron = (9 << 3) 	/* clock daemon */
		}; 

		/* open is optional */
		void syslog_open(
			const char * /*tag*/ = nullptr , 
			syslog_open_options = syslog_open_options::_null_ ,
			syslog_open_facilities  = syslog_open_facilities::log_user
		);

		void syslog_emergency(const char * format_, ...);
		void syslog_alert(const char * format_, ...);
		void syslog_critical(const char * format_, ...);
		void syslog_error(const char * format_, ...);
		void syslog_warning(const char * format_, ...);
		void syslog_notice(const char * format_, ...);
		void syslog_info(const char * format_, ...);
		void syslog_debug(const char * format_, ...);

} // dbj::log

#ifdef DBJ_LOG_TESTING
#include "test/dbj_log_test.h"
#endif

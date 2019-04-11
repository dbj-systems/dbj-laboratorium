#pragma once
#include "dbj_gpl_license.h"
/*
(c) 2019 by dbj.systems, author: dbj@dbj.org, Licence GPLv3

Please see the README.MD for base documentation 
Please see the LICENSE for the GPLv3 info

NOTE: syslog() is UNIX and is blisfully unaware of wchar_t
*/

#include <array>
#include <system_error>

namespace dbj::log {

	// SEMantic VERsioning
	constexpr inline const auto MAJOR = 0;
	constexpr inline const auto MINOR = 4;
	constexpr inline const auto PATCH = 0;

	constexpr inline const auto syslog_dgram_size = 1024U;

	extern "C" void syslog_init(const char * = nullptr /*syslog_server_ip_and_port*/);

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
	extern "C" void syslog_open(
		const char * /*tag*/ = nullptr,
		syslog_open_options = syslog_open_options::_null_,
		syslog_open_facilities = syslog_open_facilities::log_user
	);


	enum class syslog_level {
		log_emerg = 0, /* system is unusable */
		log_alert = 1, /* action must be taken immediately */
		log_crit = 2, /* critical conditions */
		log_err = 3, /* error conditions */
		log_warning = 4, /* warning conditions */
		log_notice = 5, /* normal but significant condition */
		log_info = 6, /* informational */
		log_debug = 7	 /* debug-level messages */
	};

	 extern "C" void syslog_emergency(const char * format_, ...);

	 extern "C" void syslog_alert(const char * format_, ...);

	 extern "C" void syslog_critical(const char * format_, ...);

	 extern "C" void syslog_error(const char * format_, ...);

	 extern "C" void syslog_warning(const char * format_, ...);

	 extern "C" void syslog_notice(const char * format_, ...);

	 extern "C" void syslog_info(const char * format_, ...);

	 extern "C" void syslog_debug(const char * format_, ...);

} // dbj::log

// macros are bad but still very effective for decoupling
#if defined(DBJ_SYSLOG)

// this is for std::error_code
#define DBJ_LOG_STD_ERR(err,msg_if_err) \
do { if (err) ::dbj::log::syslog_error( "%s, %s", err.message().c_str(), DBJ_ERR_PROMPT(msg_if_err) ); } while(false)

#define DBJ_LOG_ERR(...) ::dbj::log::syslog_error( __VA_ARGS__ ) 
#define DBJ_LOG_CRT(...) ::dbj::log::syslog_critical( __VA_ARGS__ ) 
#define DBJ_LOG_LRT(...) ::dbj::log::syslog_alert( __VA_ARGS__ ) 
#define DBJ_LOG_WRG(...) ::dbj::log::syslog_warning( __VA_ARGS__ ) 
#define DBJ_LOG_MCY(...) ::dbj::log::syslog_emergency( __VA_ARGS__ ) 
#define DBJ_LOG_DBG(...) ::dbj::log::syslog_debug( __VA_ARGS__ ) 
#define DBJ_LOG_INF(...) ::dbj::log::syslog_info( __VA_ARGS__ ) 
#define DBJ_LOG_NTC(...) ::dbj::log::syslog_notice( __VA_ARGS__ ) 
#else
#define DBJ_LOG_ERR __noop
#define DBJ_LOG_CRT __noop
#define DBJ_LOG_LRT __noop
#define DBJ_LOG_WRG __noop
#define DBJ_LOG_MCY __noop
#define DBJ_LOG_DBG __noop
#define DBJ_LOG_INF __noop
#define DBJ_LOG_NTC __noop
#define DBJ_LOG_STD_ERR __noop
#endif

#ifdef DBJ_LOG_TESTING
#include "test/dbj_log_test.h"
#endif


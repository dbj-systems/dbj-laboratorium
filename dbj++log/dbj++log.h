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

#include <array>

namespace dbj::log {

	constexpr inline const auto syslog_dgram_size = 1024U;

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

	extern "C" void the_great_decoupler(syslog_level, const char *);

	template<typename ... A>
	inline void syslog_call(syslog_level level_, const char * format_, A ... args)
	{
		// this static is locking, no mutex necessary
		static std::array<char, syslog_dgram_size> message_{ { 0 } };
		message_.fill(0); // zero the buffer
		auto kontrol = std::snprintf(message_.data(), message_.size(), args ...);
		_ASSERTE(kontrol > 1);
		the_great_decoupler(level_, message_.data());
	}

	template<typename ... A>
	inline void syslog_emergency(const char * format_, A ... args)
	{
		syslog_call(syslog_level::log_emerg, format_, args ...);
	}

	template<typename ... A>
	inline void syslog_alert(const char * format_, A ... args)
	{
		syslog_call(syslog_level::log_alert, format_, args ...);
	}

	template<typename ... A>
	inline void syslog_critical(const char * format_, A ... args)
	{
		syslog_call(syslog_level::log_crit, format_, args ...);
	}

	template<typename ... A>
	inline void syslog_error(const char * format_, A ... args)
	{
		syslog_call(syslog_level::log_err, format_, args ...);
	}

	template<typename ... A>
	inline void syslog_warning(const char * format_, A ... args)
	{
		syslog_call(syslog_level::log_warning, format_, args ...);
	}

	template<typename ... A>
	inline void syslog_notice (const char * format_, A ... args)
	{
		syslog_call(syslog_level::log_notice, format_, args ...);
	}

	template<typename ... A>
	inline void syslog_info(const char * format_, A ... args)
	{
		syslog_call(syslog_level::log_info, format_, args ...);
	}

	template<typename ... A>
	inline void syslog_debug(const char * format_, A ... args)
	{
		syslog_call(syslog_level::log_debug, format_, args ...);
	}

} // dbj::log

#ifdef DBJ_LOG_TESTING
#include "test/dbj_log_test.h"
#endif

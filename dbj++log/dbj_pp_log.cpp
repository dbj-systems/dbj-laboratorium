/*
I really do not like libraries with cpp files
but this time I need to completely decouple from an
inherently C *and* UNIX like lib
*/
#include "dbj++log.h"
#include "syslog/syslog.h"

namespace dbj::log {

		void syslog_init(const char * ip_and_port )
		{
			init_syslog(ip_and_port == nullptr? NULL : ip_and_port);
		}

		/* open is optional */
		void syslog_open(
			const char *tag_ ,
			syslog_open_options  option /*= syslog_open_options::_null_*/,
			syslog_open_facilities facility /*= syslog_open_facilities::log_user*/
		) 
		{
			int opt_1 = (int)option;
			int opt_2 = (int)facility;

			openlog((char*)(tag_ == nullptr ? NULL : tag_), opt_1, opt_2);
		}

		void syslog_emergency(const char *format_, ...)
		{
			if (!is_syslog_initialized()) return;
			va_list args_;
			va_start(args_, format_);
			syslog(LOG_EMERG, format_, args_);
			va_end(args_);
		}
		void syslog_alert(const char *format_, ...)
		{
			if (!is_syslog_initialized()) return;
			va_list args_;
			va_start(args_, format_);
			syslog(LOG_ALERT, format_, args_);
			va_end(args_);
		}

		void syslog_critical(const char *format_, ...)
		{
			if (!is_syslog_initialized()) return;
			va_list args_;
			va_start(args_, format_);
			syslog(LOG_CRIT, format_, args_);
			va_end(args_);
		}

		void syslog_error(const char *format_, ...)
		{
			if (!is_syslog_initialized()) return;
			va_list args_;
			va_start(args_, format_);
			syslog(LOG_ERR, format_, args_);
			va_end(args_);
		}

		void syslog_warning(const char *format_, ...)
		{
			if (!is_syslog_initialized()) return;
			va_list args_;
			va_start(args_, format_);
			syslog(LOG_WARNING, format_, args_);
			va_end(args_);
		}

		void syslog_notice(const char *format_, ...)
		{
			if (!is_syslog_initialized()) return;
			va_list args_;
			va_start(args_, format_);
			syslog(LOG_NOTICE, format_, args_);
			va_end(args_);
		}

		void syslog_info(const char *format_, ...)
		{
			if (!is_syslog_initialized()) return;
			va_list args_;
			va_start(args_, format_);
			syslog(LOG_INFO, format_, args_);
			va_end(args_);
		}

		void syslog_debug(const char *format_, ...)
		{
			if (!is_syslog_initialized()) return;
			va_list args_;
			va_start(args_, format_);
			syslog(LOG_DEBUG, format_, args_);
			va_end(args_);
		}
} // dbj::log
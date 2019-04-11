/*
I really do not like libraries with cpp files
but this time I need to completely decouple from an
inherently C *and* UNIX like lib.
By hiding local includes here, I decouple user from them too ...
*/
#include <dbj++/core/dbj_crt.h>
#include "dbj++log.h"
#include "syslog/syslog.h"
#include <cstdio>

namespace dbj::log {

		extern "C" void syslog_init(const char * ip_and_port )
		{
			init_syslog(ip_and_port == nullptr? NULL : ip_and_port);
		}

		/* open is optional */
		extern "C"  void syslog_open(
			const char *tag_ ,
			syslog_open_options  option /*= syslog_open_options::_null_*/,
			syslog_open_facilities facility /*= syslog_open_facilities::log_user*/
		) 
		{
			int opt_1 = (int)option;
			int opt_2 = (int)facility;

			openlog((char*)(tag_ == nullptr ? NULL : tag_), opt_1, opt_2);
		}

		template<typename ... A>
		void syslog_call(syslog_level level_, const char * format_, A ... args)
		{
			// this static is locking, no mutex necessary
			static std::array<char, syslog_dgram_size> message_{ { 0 } };

			if (!is_syslog_initialized()) return;

			message_.fill(0); // zero the buffer
			auto kontrol = std::snprintf(message_.data(), message_.size(), format_, args ...);
			_ASSERTE(kontrol > 1);
			vsyslog((int)level_, message_.data());
		}

		
		 void syslog_emergency(const char * format_, ...)
		{
			DBJ_AUTO_LOCK;
			va_list ap;
			va_start(ap, format_);
			syslog_call(syslog_level::log_emerg, format_, ap);
			va_end(ap);
		 }

		
		 void syslog_alert(const char * format_, ...)
		{
			DBJ_AUTO_LOCK;
			va_list ap;
			va_start(ap, format_);
			syslog_call(syslog_level::log_alert, format_, ap);
			va_end(ap);
		 }

		
		 void syslog_critical(const char * format_, ...)
		{
			DBJ_AUTO_LOCK;
			va_list ap;
			va_start(ap, format_);
			syslog_call(syslog_level::log_crit, format_, ap);
			va_end(ap);
		 }

		
		 void syslog_error(const char * format_, ...)
		{
			DBJ_AUTO_LOCK;
			va_list ap;
			va_start(ap, format_);
			syslog_call(syslog_level::log_err, format_, ap);
			va_end(ap);
		 }

		
		 void syslog_warning(const char * format_, ...)
		{
			DBJ_AUTO_LOCK;
			va_list ap;
			va_start(ap, format_);
			syslog_call(syslog_level::log_warning, format_, ap);
			va_end(ap);
		 }

		
		 void syslog_notice(const char * format_, ...)
		{
			DBJ_AUTO_LOCK;
			va_list ap;
			va_start(ap, format_);
			syslog_call(syslog_level::log_notice, format_, ap);
			va_end(ap);
		 }

		 void syslog_info(const char * format_, ... )
		{
			DBJ_AUTO_LOCK;
			va_list ap;
			va_start(ap, format_);
			syslog_call(syslog_level::log_info, format_, ap);
			va_end(ap);
		}

		 void syslog_debug(const char * format_, ...)
		{
			DBJ_AUTO_LOCK;
			va_list ap;
			va_start(ap, format_);
			syslog_call(syslog_level::log_debug, format_, ap);
			va_end(ap);
		 }

} // dbj::log




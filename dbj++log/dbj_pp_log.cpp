/*
I really do not like libraries with cpp files
but this time I need to completely decouple from an
inherently C *and* UNIX like lib.
By hiding local includes here, I decouple user from them too ...
*/

#include <cstdio>
//#include <crtdbg.h>
//#include <windows.h>

#include <dbj++/core/dbj++core.h>
#include <dbj++/win/dbj++win.h>
#include "dbj++log.h"
#include "syslog/syslog.h"


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

		template<typename ... T>  void  syslog_emergency(const char* format_, T ... args)
		{
			DBJ_AUTO_LOCK;
			inner::syslog_call(syslog_level::log_emerg, format_, args...);
		}

		template<typename ... T>  void  syslog_alert(const char* format_, T ... args)
		{
			DBJ_AUTO_LOCK;
			inner::syslog_call(syslog_level::log_alert, format_, args...);
		}

		template<typename ... T>  void  syslog_critical(const char* format_, T ... args)
		{
			DBJ_AUTO_LOCK;
			inner::syslog_call(syslog_level::log_crit, format_, args...);
		}

		template<typename ... T>  void  syslog_error(const char* format_, T ... args)
		{
			DBJ_AUTO_LOCK;
			inner::syslog_call(syslog_level::log_err, format_, args...);
		}


		template<typename ... T>  void  syslog_warning(const char* format_, T ... args)
		{
			DBJ_AUTO_LOCK;
			inner::syslog_call(syslog_level::log_warning, format_, args...);
		}


		template<typename ... T>  void  syslog_notice(const char* format_, T ... args)
		{
			DBJ_AUTO_LOCK;
			inner::syslog_call(syslog_level::log_notice, format_, args...);
		}

		template<typename ... T>  void  syslog_info(const char* format_, T ... args)
		{
			DBJ_AUTO_LOCK;
			inner::syslog_call(syslog_level::log_info, format_, args...);
		}

		template<typename ... T>  void  syslog_debug(const char* format_, T ... args)
		{
			DBJ_AUTO_LOCK;
			inner::syslog_call(syslog_level::log_debug, format_, args...);
		}

} // dbj::log


// handle the CTRL+C exit
static BOOL WINAPI __dbj_log_console_handler(DWORD signal) {

	if (signal == CTRL_C_EVENT) {
		// namespace galimatias, almost like cppwinrt ;)
		using dbj::chr_buf::yanb;
		using ::dbj::core::trace;
		using namespace ::dbj::win32;
		using namespace ::dbj::log;

		yanb basename_{ module_basename() };
		DBJ_LOG_INF("CTRL+C event -- from %s", basename_.data());
		trace("CTRL+C event -- from %s", basename_.data());
	}
	return TRUE;
}

// have to do it here so that caller can use it before main ...
// this is an "self executing" lambda
// this happens only once, for each process
static const auto __dbj_log_init__ = []()
{
	// namespace galimatias, almost like cppwinrt ;)
	using dbj::chr_buf::yanb;
	using ::dbj::core::trace;
	using namespace ::dbj::win32;
	using namespace ::dbj::log;

	yanb basename_{ module_basename() };
	syslog_init();
	// syslog_open_options::log_perror
	// makes use of local log file
	syslog_open(
		basename_.data() /*, syslog_open_options::log_perror*/
	);
	trace("syslog connection opened from %s", basename_.data());

	if (!SetConsoleCtrlHandler(__dbj_log_console_handler, TRUE)) {
		trace("\nERROR: Could not set console exit handler");
		perror("\nERROR: Could not set console exit handler");
	}

	return true;
}();



/*
I really do not like libraries with cpp files
but this time I need to completely decouple from an
inherently C *and* UNIX like lib.
By hiding local includes here, I decouple user from them too ...
*/

#define DBJ_INCLUDE_STD_
#include <dbj++/dbj++required.h>

// #define DBJ_WMAIN_USED <-- since we are building a library we do not need wmain()
#include <dbj++/dbj++.h>

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



} // dbj::log


// handle the CTRL+C exit
static BOOL WINAPI __dbj_log_console_handler(DWORD signal) {

	if (signal == CTRL_C_EVENT) {

		// namespace galimatias, almost like cppwinrt ;)
		using ::dbj::core::trace;
		using namespace ::dbj::win32;
		using namespace ::dbj::log;

		auto basename_{ module_basename() };
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
	using ::dbj::core::trace;
	using namespace ::dbj::win32;
	using namespace ::dbj::log;

	auto basename_{ module_basename() };
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



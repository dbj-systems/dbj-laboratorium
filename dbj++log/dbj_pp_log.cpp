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
#include <crtdbg.h>
#include <windows.h>

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



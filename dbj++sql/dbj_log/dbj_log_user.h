#pragma once
#include <string_view>
#include <dbj++/core/dbj++core.h>
#include <dbj++log/dbj++log.h>
/*
using the syslog 
*/

namespace dbj::db::err {

	using namespace std;

	// in sqlite3 basicaly there are error code constants and only 3 are not errors

	inline void log_sql_ec(std::error_code ec, string_view  log_message = "  "sv) noexcept
	{
		::dbj::buf::yanb buffer_ 
			= ::dbj::fmt::to_buff("%s %s", ec.message(), log_message);

		if (
			(ec == dbj_err_code::sqlite_ok)  ||
			(ec == dbj_err_code::sqlite_row) ||
			(ec == dbj_err_code::sqlite_done)
			)
			::dbj::log::syslog_info("%s", buffer_.data());
		else
			::dbj::log::syslog_error("%s", buffer_.data());
	}

	/*
	In SQLITE3 API
	There are only a few non-error result codes:
	SQLITE_OK, SQLITE_ROW, and SQLITE_DONE.
	The term "error code" means any result code other than these three.
	Thus there is no concept of 'warning'
	Thus from here we will simply not log them no error's.
	Thus, considerably downsizing the log size

	we do not make log if SQLITE_OK == result 
	if errors, before returning them we log them
	so when they are received or caught (if thrown)
	the full info is already in the log
	*/
	[[nodiscard]] inline
		error_code sqlite_ec(
			int sqlite_retval,
			// make it longer than 1 so that logger will not complain
			string_view  log_message = "  "sv
		)
		noexcept
	{
		::std::error_code ec = int_to_dbj_error_code(sqlite_retval);

		// _ASSERTE((int)dbj_err_code::sqlite_ok == SQLITE_OK );

		if (sqlite_retval != (int)dbj_err_code::sqlite_ok)
			log_sql_ec(ec, log_message);

		return ec;
	}

	// make and return std::errc as error_code
	// also log the message
	[[nodiscard]] inline error_code
		std_ec(
			std::errc posix_retval,
			// make it optional and alos longer 
			// than 1 so that logger will not complain
			string_view  log_message = "  "sv
		)	noexcept
	{
		// std::errc should not contain a 0
			::std::error_code ec = std::make_error_code(posix_retval);
			// each posix code is seen as 'error'
			if (0 == (int)posix_retval)
				::dbj::log::syslog_info("%s %s",ec.message().data(), log_message.data());
			else
				::dbj::log::syslog_error("%s %s", ec.message().data(), log_message.data());
		return ec;
	}
} // dbj::db::err nspace

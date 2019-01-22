#pragma once
#include "dbj_log.h"
#include "dbj_log_file.h"
/*
using the generic log for specific purposes
*/

namespace dbj::db::err {

	// in sqlite3 basicaly there are error code constants and only 3 are not errors

	inline void log_ignore_ok(std::error_code ec, string_view  log_message = "  "sv) noexcept
	{
		// we ignore any zero
		if (ec.value() == 0)
			return;

		if (ec == dbj_err_code::sqlite_ok) // actually a zero
			return;

		if (
			(ec == dbj_err_code::sqlite_row) ||
			(ec == dbj_err_code::sqlite_done)
			)
			::dbj::db::log::info(ec.message(), log_message);
		else
			::dbj::db::log::error(ec.message(), log_message);
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
	inline[[nodiscard]]
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
			log_ignore_ok(ec, log_message);

		return ec;
	}
} // dbj::db::err nspace

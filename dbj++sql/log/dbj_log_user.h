#pragma once
#include "dbj_log.h"
/*
using the generic log for specific purposes
*/

namespace dbj::db::err {

	// in sqlite3 basicaly there are error code constants and only 3 are not errors

	inline void log_ignore_ok(std::error_code ec, string_view  log_message = "  "sv) noexcept
	{
		auto decider = [&]() {
			if (ec == dbj_err_code::sqlite_ok)
				return;
			else
				if (
					(ec == dbj_err_code::sqlite_row) ||
					(ec == dbj_err_code::sqlite_done)
					)
					::dbj::db::log::info(ec.message(), log_message);
				else
					::dbj::db::log::error(ec.message(), log_message);
		};

		// hack alert: we never store the async result
		// thus they wait for each other
		// making the whole logging thing, async but sequential?
		(void)std::async(std::launch::async, [&] { decider(); });
	}

	/*
	In SQLITE3 API
	There are only a few non-error result codes:
	SQLITE_OK, SQLITE_ROW, and SQLITE_DONE.
	The term "error code" means any result code other than these three.
	Thus there is no concept of 'warning'
	Thus from here we will simply not log them no error's.
	Thus, considerably downsizing the log size

	we do not make log and return always
	if SQLITE_OK == result we do not log
	if they are errors before returning them we log them
	so when they are received or caught (if thrown)
	the full info is already in the log
	*/
	inline[[nodiscard]]
		error_code dbj_sql_err_log_get(
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

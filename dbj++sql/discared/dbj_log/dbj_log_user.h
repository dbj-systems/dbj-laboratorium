#pragma once
#include <string_view>
#include <dbj++/core/dbj++core.h>
#include <dbj++/win/dbj++win.h>
#include <dbj++log/dbj++log.h>
/*
using the syslog
*/

namespace dbj::db::err {

	using namespace ::std;
	using namespace ::dbj::log;

	inline auto dumsy_ = []() {
		syslog_init();
		syslog_open(
			"dbj++sql", syslog_open_options::log_perror
		);
		return true;
	}();

	// in sqlite3 basicaly there are error code constants and only 3 are not errors
	inline void log_error(
		std::error_code ec, 
		/* 
		not optional, please provde more info which is not delivered by the message() method
		on the error_code 
		*/
		string_view  log_message
	)
		noexcept
	{
		::dbj::chr_buf::yanb buffer_
			= ::dbj::fmt::to_buff("%s %s", ec.message().c_str(), log_message);

		if (::dbj::db::err::is_sql_not_err(ec))
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
			// not optional so calling code has
			// to provide more info than terse 
			// error_code message method does
			string_view  log_message 
		)
		noexcept
	{
		::std::error_code ec = int_to_dbj_error_code(sqlite_retval);

		// _ASSERTE((int)sqlite_err_id::sqlite_ok == SQLITE_OK );

		// avoid double checking if this is SQL LITE error
		if ( ! ::dbj::db::err::is_sql_not_err(ec))
				log_error(ec, log_message);

		return ec;
	}

	// essentially transform POSIX error code
	// into std::error_code 
	[[nodiscard]] inline error_code
		to_std_error_code(
			std::errc posix_retval,
			// force caller to provide more info
			string_view  log_message
		)	noexcept
	{
		::std::error_code ec = std::make_error_code(posix_retval);
#ifdef _DEBUG
		/* 
		the concept error_code designer has followed is 
		if error ...
		this is what is the logic of the bool operator on it
		*/
		if (ec)
		::dbj::log::syslog_error("%s %s", ec.message().data(), log_message.data());
#endif
		return ec;
	}
} // dbj::db::err nspace

#pragma once

#include "dbj_sqlite_nanolib.h"
#include "sqlite++.h"

/*
no exception but return values

return value type can have or not have, sqlite status code and std::errc code

*/

namespace dbj::sql
{
	using namespace std;


	using buffer = typename v_buffer ;
	using buffer_type = typename buffer::buffer_type ;

	enum class status_code : int
	{
		/*
			https://sqlite.org/c3ref/c_abort.html
			note: must be carefull not to clash with sqlite3 # defines
			*/
		sqlite_ok =				SQLITE_OK		  ,	/* successful result */
		sqlite_error =			SQLITE_ERROR      , /* generic error */
		sqlite_internal =		SQLITE_INTERNAL   , /* internal logic error in sqlite */
		sqlite_perm =			SQLITE_PERM       , /* access permission denied */
		sqlite_abort =			SQLITE_ABORT      , /* callback routine requested an abort */
		sqlite_busy =			SQLITE_BUSY       , /* the database file is locked */
		sqlite_locked =			SQLITE_LOCKED     , /* a table in the database is locked */
		sqlite_nomem =			SQLITE_NOMEM      , /* a malloc() failed */
		sqlite_readonly =		SQLITE_READONLY   , /* attempt to write a readonly database */
		sqlite_interrupt =		SQLITE_INTERRUPT  , /* operation terminated by sqlite= 3 _interrupt()*/
		sqlite_ioerr =			SQLITE_IOERR      , /* some kind of disk i/o error occurred */
		sqlite_corrupt =		SQLITE_CORRUPT    , /* the database disk image is malformed */
		sqlite_notfound =		SQLITE_NOTFOUND   , /* unknown opcode in sqlite= 3 _file_control() */
		sqlite_full =			SQLITE_FULL       , /* insertion failed because database is full */
		sqlite_cantopen =		SQLITE_CANTOPEN   , /* unable to open the database file */
		sqlite_protocol =		SQLITE_PROTOCOL   , /* database lock protocol error */
		sqlite_empty =			SQLITE_EMPTY      , /* internal use only */
		sqlite_schema =			SQLITE_SCHEMA     , /* the database schema changed */
		sqlite_toobig =			SQLITE_TOOBIG     , /* string or blob exceeds size limit */
		sqlite_constraint =		SQLITE_CONSTRAINT , /* abort due to constraint violation */
		sqlite_mismatch =		SQLITE_MISMATCH   , /* data type mismatch */
		sqlite_misuse =			SQLITE_MISUSE     , /* library used incorrectly */
		sqlite_nolfs =			SQLITE_NOLFS      , /* uses os features not supported on host */
		sqlite_auth =			SQLITE_AUTH       , /* authorization denied */
		sqlite_format =			SQLITE_FORMAT     , /* not used */
		sqlite_range =			SQLITE_RANGE      , /* = 2 nd parameter to sqlite= 3 _bind out of range */
		sqlite_notadb =			SQLITE_NOTADB     , /* file opened that is not a database file */
		sqlite_notice =			SQLITE_NOTICE     , /* notifications from sqlite= 3 _log() */
		sqlite_warning =		SQLITE_WARNING    , /* warnings from sqlite= 3 _log() */
		sqlite_row =			SQLITE_ROW        , /* sqlite= 3 _step() has another row ready */
		sqlite_done =			SQLITE_DONE        /* sqlite= 3 _step() has finished executing */
	};                          // dbj_status_code/*

	/*
	this is the sqlite3 logic 
	not all error codes mean errors
	*/
	inline constexpr bool is_sqlite_error (status_code const & sc_ ) noexcept
	{
		switch (sc_) {
		case status_code::sqlite_ok:
		case status_code::sqlite_row:
		case status_code::sqlite_done:
			return false;
		default:
			return true;
		};
	}

	/*-----------------------------------------------------------------
	DBJ DB status type -- what is the status after the call

	contains any combination of sqlite3 status codes and std::errc codes
	std::nullopt signals the absence

	absence or presence is the state consumers/callers use 
	see the bool operator
	*/
	struct dbj_db_status_type final
	{
		using sqlite_status_type = optional< ::dbj::sql::status_code >;
		using std_errc_type = optional< std::errc >;
		
		sqlite_status_type	sqlite_status_id;
		std_errc_type		std_errc;

		dbj_db_status_type() { clear();  };

		dbj_db_status_type(
			sqlite_status_type sqlite_status_, 	
			std_errc_type std_errc_ = nullopt
			)
			: sqlite_status_id(sqlite_status_), std_errc(std_errc_)
		{
		}

		/*
		from sqlite3 code int's are coming
		*/
		dbj_db_status_type(
			int sqlite_status_,
			std_errc_type std_errc_ = nullopt
			)
			: sqlite_status_id(::dbj::sql::status_code(sqlite_status_))
			, std_errc(std_errc_)
		{
		}

		operator bool() const
		{
			if (sqlite_status_id != nullopt)
			{
			if (is_sqlite_error(*sqlite_status_id))
				return true;
            }
				
			if (std_errc != nullopt)  return true;

			return false; // not in an error state
		}

		void clear( )
		{
			sqlite_status_id = nullopt;
			std_errc = nullopt;
		}

		/*
		The sqlite3_errstr() interface returns the English-language text that
		describes the result code, as UTF-8. Memory to hold the error message
		string is managed internally and must not be freed by the application.
		*/
		optional<buffer_type> sql_err_message() noexcept
		{
			// not set
			if (!sqlite_status_id) return nullopt;

			status_code ev = *sqlite_status_id;
			if (const char* mp_ = ::sqlite::sqlite3_errstr(int(ev)); mp_ != nullptr)
			{
				return buffer::make(mp_);
			}
			else
			{
				return buffer::make("Unknown SQLITE status code");
			}
		}

		// essentially transform POSIX error code
		// into std::error_code message
		optional<buffer_type> std_err_message()
		{
			if (!std_errc) return nullopt;
			std::errc posix_retval = * std_errc;
			::std::error_code ec = std::make_error_code(posix_retval);
			return buffer::make(ec.message().c_str());
		}

		static buffer_type to_buffer(dbj_db_status_type & status_ ) 
		{
			buffer_type buffy_ = buffer::make(0xFF + 0xFF); // 512 aka POSIX BUFSIZ

			auto sql_status_id = status_.sqlite_status_id;
			auto sql_status_message_ = status_.sql_err_message();

			auto std_errc_id = status_.std_errc;
			auto std_status_message_ = status_.std_err_message();

			int rez_ = std::snprintf(
				buffy_.data(), buffy_.size(),
				"sqlite3 id:%d, message:%s\nstd::errc id:%d, message:%s",
				(sql_status_id ? int(*sql_status_id) : 0 ),
				(sql_status_message_ ? (*sql_status_message_).data() : "SQLITE_OK" ),
				(std_errc_id ? int(*std_errc_id) : 0 ),
				(std_status_message_ ? (*std_status_message_).data() : "POSIX_OK" )
			);

			if (rez_ < 0 )
				dbj_terror("std::snprintf() failed", __FILE__, __LINE__);

			return buffy_;
		}

	}; // dbj_db_status_type
} // namespace dbj::sqlite

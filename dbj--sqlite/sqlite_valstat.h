#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#ifndef _DBJ_SQLITE_STATUS_INC_
#define _DBJ_SQLITE_STATUS_INC_

#if 0
#ifndef _DBJ_STATUS_INC_
#include "dbj--nanolib/dbj++status.h"
// #include "D:\DEVL\GitHub\dbj--nanolib\dbj++status.h"
#endif
#endif

/*
to use dbj nanolib return type concept for
another module/library one needs to provide

1. value type
	if user defined type, the only requirement is  that 'optional' can use it
2. status code type, example: std::errc
3. "code to message" function, of the signature
	 buffer_type ( code_type )
4. "code to int" function of the signature
	int ( code_type ) ;
5.	"category name" function
	example: constexpr auto category_name() { return "posix"; }

	With those user next defines the 'service' for making the required
	return and/or status types, example, for 'posix' evailable in dbj::nanolib :

		template <typename T>
	using posix_retval_service = return_type_service<
		// value type
		T,
		//  code type
		std::errc,
		// code to message
		posix::code_to_message,
		// code to int
		posix::code_to_int,
		// category name
		posix::category_name>;

Where `return_type_service` is a template matching the following synopsis:
*/
//	template <
//		typename value_type_,
//		typename code_type_param, /* has to be castable to int */
//v_buffer::buffer_type(*code_to_message)(code_type_param),
//int (*code_to_int)(code_type_param),
//char const* (*category_name)()
//	>
//	struct return_type_service final
//{
//	using type = return_type_service;
//	using value_type = value_type_;
//	using return_type = pair< optional<value_type>, optional<status_type> >;
//	using code_type = code_type_param;
//	constexpr static inline char const* category = category_name();
//
//	static status_type make_status(code_type code, char const* file, long line);
//
//	/* just make info status */
//	static status_type make_status(char const* information, char const* file, long line);
//
//	// no value, status present,  means error
//	// --> { { } , { status } }
//	static return_type make_error(status_type status_);
//
//	// value, no status is normal return
//	// status part is redundant --> { { value } , { } }
//	static return_type make_ok(value_type /*const&*/ value_);
//
//	// both status and value we cann "info return"
//	// --> { { value } , { status } }
//	static return_type make_full(value_type /*const&*/ value_, status_type status_);
//}; // return_type_service

namespace dbj::sql
{
	// using namespace std;
	using buffer = typename dbj::nanolib::v_buffer;
	using buffer_type  = typename buffer::buffer_type;

	#define DBJ_SQL_INNER_NS

	constexpr const char* category_name() { return "sqlite3"; }

		enum class sqlite_status_code : int
		{
			/*
				https://sqlite.org/c3ref/c_abort.html
				note:
				this must match with sqlite3 # defines
			*/
			sqlite_ok = SQLITE_OK,	/* successful result */
			sqlite_error = SQLITE_ERROR, /* generic error */
			sqlite_internal = SQLITE_INTERNAL, /* internal logic error in sqlite */
			sqlite_perm = SQLITE_PERM, /* access permission denied */
			sqlite_abort = SQLITE_ABORT, /* callback routine requested an abort */
			sqlite_busy = SQLITE_BUSY, /* the database file is locked */
			sqlite_locked = SQLITE_LOCKED, /* a table in the database is locked */
			sqlite_nomem = SQLITE_NOMEM, /* a malloc() failed */
			sqlite_readonly = SQLITE_READONLY, /* attempt to write a readonly database */
			sqlite_interrupt = SQLITE_INTERRUPT, /* operation terminated by sqlite= 3 _interrupt()*/
			sqlite_ioerr = SQLITE_IOERR, /* some kind of disk i/o error occurred */
			sqlite_corrupt = SQLITE_CORRUPT, /* the database disk image is malformed */
			sqlite_notfound = SQLITE_NOTFOUND, /* unknown opcode in sqlite= 3 _file_control() */
			sqlite_full = SQLITE_FULL, /* insertion failed because database is full */
			sqlite_cantopen = SQLITE_CANTOPEN, /* unable to open the database file */
			sqlite_protocol = SQLITE_PROTOCOL, /* database lock protocol error */
			sqlite_empty = SQLITE_EMPTY, /* internal use only */
			sqlite_schema = SQLITE_SCHEMA, /* the database schema changed */
			sqlite_toobig = SQLITE_TOOBIG, /* string or blob exceeds size limit */
			sqlite_constraint = SQLITE_CONSTRAINT, /* abort due to constraint violation */
			sqlite_mismatch = SQLITE_MISMATCH, /* data type mismatch */
			sqlite_misuse = SQLITE_MISUSE, /* library used incorrectly */
			sqlite_nolfs = SQLITE_NOLFS, /* uses os features not supported on host */
			sqlite_auth = SQLITE_AUTH, /* authorization denied */
			sqlite_format = SQLITE_FORMAT, /* not used */
			sqlite_range = SQLITE_RANGE, /* = 2 nd parameter to sqlite= 3 _bind out of range */
			sqlite_notadb = SQLITE_NOTADB, /* file opened that is not a database file */
			sqlite_notice = SQLITE_NOTICE, /* notifications from sqlite= 3 _log() */
			sqlite_warning = SQLITE_WARNING, /* warnings from sqlite= 3 _log() */
			sqlite_row = SQLITE_ROW, /* sqlite= 3 _step() has another row ready */
			sqlite_done = SQLITE_DONE        /* sqlite= 3 _step() has finished executing */
		};                          // dbj_status_code/*

			/*
			The sqlite3_errstr() interface valstat the English-language text that
			describes the result code, as UTF-8. Memory to hold the error message
			string is managed internally and must not be freed by the application.

			function argument is int so that native sqlite3 return values can be
			used also
			*/
		inline buffer_type err_message_sql(int sqlite_return_value)  noexcept
		{
			if (const char* mp_ = ::sqlite::sqlite3_errstr(sqlite_return_value); mp_ != nullptr)
				return buffer::make(mp_);
			else
				return buffer::make("Unknown SQLITE status code");
		}

		// this overload is required by nanolib valstat service
		inline buffer_type err_message_sql(sqlite_status_code code_)  noexcept
		{
			return err_message_sql(int(code_));
		}
		
		inline int code_to_int(sqlite_status_code code_)  noexcept
		{
			return int(code_);
		}

		/*
		this is the sqlite3 logic: not all codes are errors
		*/
		inline constexpr bool is_sqlite_error(sqlite_status_code sc_) noexcept
		{
			switch (sc_) {
			case sqlite_status_code::sqlite_ok:
			case sqlite_status_code::sqlite_row:
			case sqlite_status_code::sqlite_done:
				return false;
			default:
				return true;
			};
		}

		// sqlite api returns int status codes basically
		inline constexpr bool is_sqlite_error(int sqlite3_result_) noexcept
		{
			return is_sqlite_error(sqlite_status_code (sqlite3_result_));
		}

	template <typename T>
	using sqlite3_valstat_trait_template = ::dbj::nanolib::valstat_trait<
		// value type
		T,
		//  code type
		DBJ_SQL_INNER_NS sqlite_status_code,
		// code to message
		DBJ_SQL_INNER_NS err_message_sql,
		// code to int
		DBJ_SQL_INNER_NS code_to_int,
		// category name
		DBJ_SQL_INNER_NS category_name>;

	/*
	here is the important part:
	because value/status concept is flexible it is powerfull
	and that put together makes it complex to setup 
	not complex to use

	So. In the context of the domain of this library we need sqlite3 codes
	to be values returned, often but not always.

	So, we will create that type here 
	*/
	using sqlite3_valstat_trait = typename sqlite3_valstat_trait_template<DBJ_SQL_INNER_NS sqlite_status_code>;
	using dbj_sql_valstat = typename sqlite3_valstat_trait::return_type;


	/*
	in this solution domian we do not use always "if no value then error" logic
	we rather always check with the method above if the value returned
	is treated by sqlite3 as error
	*/
	inline constexpr bool is_error
	(dbj_sql_valstat const& sqlite3_rt)
	{
		auto optival = sqlite3_rt.first;
		// no value so def. an error
		if (!optival) return true;
		// yes value, check is it an error by sqlite3 logic
		return is_sqlite_error(optival.value());
	}
	/*
	remember the return structure from the above trait is :
	{{ optional sqlite_status_code },{ optional status message }}
	here we make conveience macros using the valstat above
	and the dbj nanolib generic macros
	*/
#define DBJ_SQL_VALSTAT_ERR(V_) DBJ_STATVAL_ERR(dbj::sql::sqlite3_valstat_trait, V_)
#define DBJ_SQL_VALSTAT_OK(V_) DBJ_STATVAL_OK(dbj::sql::sqlite3_valstat_trait, V_)

	/*
	make vlastat from slqite3 API call, returned int rezult
	here we avoid making fully populated pair of options thus
	allowing callers to use the default paradigm

	auto [v,s] = func() ;
	if (!v) ...then error...
	*/
	auto make_dbj_sql_valstat = [] 
	( int sqlite_3_result_value_ , const char * file_, long line_ ) 
	{
		using status_type =
			::dbj::sql::DBJ_SQL_INNER_NS sqlite_status_code;
		
		status_type ssc_ =
			static_cast<status_type>(sqlite_3_result_value_);

		if (is_sqlite_error(ssc_)) {
			return sqlite3_valstat_trait::make_error(
				sqlite3_valstat_trait::make_status( ssc_, file_, line_  )
			);
		}
		else {
			return sqlite3_valstat_trait::make_ok(ssc_);
		}
	};

#define DBJ_SQL_VALSTAT( R_ ) make_dbj_sql_valstat(R_, __FILE__, __LINE__ ) 

	// here we make the OK statval object so we do not re-make it each time we need it
	inline const dbj_sql_valstat sqlite3_ok_statval 
		= sqlite3_valstat_trait::make_ok( sqlite_status_code::sqlite_ok) ;

	/* do not go overboard with macros */
	auto print_on_sql_error = [](dbj_sql_valstat const& S_)
	{
		if (::dbj::sql::is_error(S_)) 
		{ 
			DBJ_FPRINTF(stderr, DBJ_FG_RED_BOLD "\nERROR Status\n%s\n\n" DBJ_RESET, S_.second->data()); 
		}
	};

#define DBJ_RETURN_ON_SQL_ERROR(S_) dbj::sql::print_on_sql_error(S_); return 

} // namespace dbj::sql


#endif // !_DBJ_SQLITE_STATUS_INC_

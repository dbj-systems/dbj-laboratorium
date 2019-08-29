#pragma once
#include <system_error>
#include <string>
#include <string_view>

/*
DBJ++ERR errors handling concept for the dbj++sql
-------------------------------------------------
*/

[[noreturn]] inline void dbj_terror
(const char * msg_, const char * file_, const int line_)
{
	_ASSERTE(msg_ && file_ && line_);
	std::fprintf(stderr, "\n\ndbj++sql Terminating error:%s\n%s (%d)", msg_, file_, line_);
	::exit(EXIT_FAILURE);
}

namespace dbj::db::err
{
	using namespace ::std::string_view_literals;
	using ::std::string;
	using ::std::wstring;

#pragma region utils

#ifdef _MSC_VER

	// (c) dbjdbj 2019 JAN
	struct last_win_err final
	{
		~last_win_err() {
			// make sure this is done
			// windows lore says this is required
			::SetLastError(0);
		}

		// make error code from win32 err int code 
		// POLICY: last WIN32 error is obtained on each call
		operator std::error_code() const noexcept {
			return std::error_code(::GetLastError(), std::system_category());
		}

	};

	inline std::error_code last_win_ec() {
		return static_cast<std::error_code>(last_win_err{});
	}
#endif // _MSC_VER

#pragma endregion

	enum class sqlite_err_id
	{
		/*
		https://sqlite.org/c3ref/c_abort.html
		note: must be carefull not to clash with sqlite3 # defines
		*/
		sqlite_ok = 0,   /* successful result */
		/* beginning-of-error-codes */
		sqlite_error = 1,   /* generic error */
		sqlite_internal = 2,   /* internal logic error in sqlite */
		sqlite_perm = 3,   /* access permission denied */
		sqlite_abort = 4,   /* callback routine requested an abort */
		sqlite_busy = 5,   /* the database file is locked */
		sqlite_locked = 6,   /* a table in the database is locked */
		sqlite_nomem = 7,   /* a malloc() failed */
		sqlite_readonly = 8,   /* attempt to write a readonly database */
		sqlite_interrupt = 9,   /* operation terminated by sqlite= 3 ,_interrupt()*/
		sqlite_ioerr = 10,   /* some kind of disk i/o error occurred */
		sqlite_corrupt = 11,   /* the database disk image is malformed */
		sqlite_notfound = 12,   /* unknown opcode in sqlite= 3 ,_file_control() */
		sqlite_full = 13,   /* insertion failed because database is full */
		sqlite_cantopen = 14,   /* unable to open the database file */
		sqlite_protocol = 15,   /* database lock protocol error */
		sqlite_empty = 16,   /* internal use only */
		sqlite_schema = 17,   /* the database schema changed */
		sqlite_toobig = 18,   /* string or blob exceeds size limit */
		sqlite_constraint = 19,   /* abort due to constraint violation */
		sqlite_mismatch = 20,   /* data type mismatch */
		sqlite_misuse = 21,   /* library used incorrectly */
		sqlite_nolfs = 22,   /* uses os features not supported on host */
		sqlite_auth = 23,   /* authorization denied */
		sqlite_format = 24,   /* not used */
		sqlite_range = 25,   /* = 2 ,nd parameter to sqlite= 3 ,_bind out of range */
		sqlite_notadb = 26,   /* file opened that is not a database file */
		sqlite_notice = 27,   /* notifications from sqlite= 3 ,_log() */
		sqlite_warning = 28,   /* warnings from sqlite= 3 ,_log() */
		sqlite_row = 100,  /* sqlite= 3 ,_step() has another row ready */
		sqlite_done = 101    /* sqlite= 3 ,_step() has finished executing */
	}; // sqlite_err_id
} // dbj::db::err 

// system_error fwk requires api private enums to be
// registered so it can be used by the fwk
namespace std
{
	template <>
	struct is_error_code_enum<::dbj::db::err::sqlite_err_id>
		: public true_type {};
}

namespace dbj::db::err {

	class dbj_err_category final
		: public std::error_category
	{
	public:
		[[nodiscard]] virtual const char* name()  const noexcept {
			return "dbj_sqlite_errors";
		}

/*
one of the main mistakes in C++11  system_error design is that this
method returns std::string
*/
		[[nodiscard]] virtual std::string message(int ev) const noexcept {
/*
The sqlite3_errstr() interface returns the English-language text that 
describes the result code, as UTF-8. Memory to hold the error message 
string is managed internally and must not be freed by the application.
*/
			if (const char * mp_ = ::sqlite::sqlite3_errstr(ev)
				; mp_ != nullptr)
			{
				return mp_;
			}
				return "Unknown error";
		}

		// OPTIONAL: Allow generic error conditions 
		// to be compared to this error domain aka category
		[[nodiscard]] virtual std::error_condition
			default_error_condition(int c)
			const noexcept override final
		{
		// No mapping for now
		return std::error_condition(c, *this);
		}
	};

	inline const std::error_category& get_dbj_err_category()
	{
		static dbj_err_category category_;
		return category_;
	}

// a bit if a hack
// we can do this because our error codes enum 
// do match sqlite3 error constants
	inline std::error_code int_to_dbj_error_code(int sqlite_retval) {
		// careful! this is DEBUG only
		_ASSERTE(
			(sqlite_retval >= (int)sqlite_err_id::sqlite_ok) &&
			(sqlite_retval <= (int)sqlite_err_id::sqlite_done)
		);

		return
		::std::error_code(sqlite_retval, get_dbj_err_category());
	}

	inline std::error_code make_error_code(sqlite_err_id e)
	{
		return std::error_code(
			static_cast<int>(e),
			get_dbj_err_category());
	}

	inline  std::error_condition make_error_condition(sqlite_err_id e)
	{
		return std::error_condition(
			static_cast<int>(e),
			get_dbj_err_category());
	}

	// ok, done and row are 3 codes not considered as errors in sqlite3 
	inline bool is_sql_not_err ( std::error_code ec_ ) 
	{
		static std::error_code ok_{ sqlite_err_id::sqlite_ok };
		static std::error_code done_{ sqlite_err_id::sqlite_done };
		static std::error_code row_{ sqlite_err_id::sqlite_row };
		return ((ec_ == ok_) || (ec_ == done_ ) || (ec_ == row_ )) ;
	}
	/*
	inline bool is_sql_err_done( std::error_code ec_ ) 
	{
		static std::error_code done_{ sqlite_err_id::sqlite_done };
		return ec_ == done_;
	}

	inline bool is_sql_err_row( std::error_code ec_ ) 
	{
		static std::error_code row_{ sqlite_err_id::sqlite_row };
		return ec_ == row_;
	}
	*/
#pragma region P1095
/* 
   for details and reasoning see the 
   SG14 (the GameDev & low latency ISO C++ working group)
   white paper P1095 by Nial Douglass 

	dbj++err conforming function is this

	[[nodiscard]] auto conforming_api () noexcept
	 -> dbj_db_return_type<int>
	 {
          return sucess(42) ;
		  // error retval example
		  return failure(0, sqlite_err_id::bad_argument ) ;
	 }

	 simple usage is :

	 auto [v,e] = conforming_api() ;

	 to help using the above use the things bellow
*/

	template<typename T>
	using dbj_err_return_type = std::pair<T, std::error_code>;

    // #define dbj_db_fails(vt,et) -> std::pair<vt,et>

		template<typename T>
		auto failure(T v, std::errc e_) 
		-> dbj_err_return_type<T> 
		{
			return std::make_pair( v, std::make_error_code(e_) );
		}

		template<typename T>
		auto failure(T v, std::error_code e_) 
			-> dbj_err_return_type<T>
		{
			return std::make_pair( v, e_ );
		}

		template<typename T>
		auto failure(T v, std::error_condition en_) 
			-> dbj_err_return_type<T>
		{
			return std::pair{ v, std::make_error_code(en_) };
		}
		
		template<typename T>
		auto succes(T v) 
			-> dbj_err_return_type<T>
		{
			return std::pair{ v, sqlite_err_id::sqlite_ok };
		};
#pragma endregion
} // dbj::db::err

//eof
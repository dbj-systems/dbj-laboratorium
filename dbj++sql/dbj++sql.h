#pragma once
#include "stdafx.h"
#include "sqlite++.h"
#include "./err/dbj_db_err.h"
#include "./dbj_log/dbj_log_user.h"
#include <dbj++/core/dbj++core.h>
#ifndef DBJ_VERIFY
#define DBJ_VERIFY_(x, file, line ) if (false == x ) ::dbj::db::dbj_terror( #x ", failed", file, line )
#define DBJ_VERIFY(x) DBJ_VERIFY_(x,__FILE__,__LINE__)
#endif

namespace dbj::db {

	// intelisense goes berserk on this -- using namespace ::std;
	// using namespace ::sqlite;
	using namespace ::std::literals::string_view_literals;
	using namespace ::dbj::db::err;


	// constexpr inline auto version = "1.0.0"sv;
	// core tests moved to core_tests.h
	// also with advice on u8 string literals 
	// constexpr inline auto version = "1.0.1"sv;
	// raised on 2019-01-20 to "1.1.0"
    // what's new
    // dbj err concept
	constexpr inline auto version = "1.1.0"sv;

	/* 
	bastardized version of Keny Kerr's unique_handle 
	dbj's version can not be copied or moved
	it is as simple as that ;)
	and -- it is also adorned with std:error_code returns
	so it is resilient *and* does not throw

	see the traits after this class to understand their
	required interface
	*/
	template <typename handle_trait>
	struct unique_handle final
	{
		using trait = handle_trait;
		using type = unique_handle;
		using pointer = typename trait::pointer;

		// by default there is this ctor only
		explicit unique_handle(pointer value = handle_trait::invalid()) noexcept :
			m_value{ value }
		{
		}

		// no copy no move
		// pass it as reference to/from functions
		unique_handle(unique_handle const & other_) = delete;
		auto operator=(unique_handle const & other_)->unique_handle & = delete;
		unique_handle(unique_handle && other) noexcept = delete;
		auto operator=(unique_handle && other) noexcept->unique_handle & = delete;

		~unique_handle() noexcept
		{
			// if error 
			// all is already logged 
			// relax, chill
			auto ec = close();
		}

		explicit operator bool() const noexcept
		{
			return m_value != handle_trait::invalid();
		}

		auto get() const noexcept -> pointer
		{
			return m_value;
		}

		auto get_address_of() const noexcept -> pointer *
		{
			_ASSERTE(!*this);
			return &m_value;
		}

		auto release() const noexcept -> pointer
		{
			auto value = m_value;
			m_value = handle_trait::invalid();
			return value;
		}

		auto reset(error_code & ec , pointer value = handle_trait::invalid()) const -> bool
		{
			if (m_value != value)
			{
				ec.clear();
				// caller must inspect
				ec = close(); 
					m_value = value;
			}
			// API designer wants to return a bool 
			// and nothing else
			return static_cast<bool>(*this);
		}
	private:

		mutable	pointer m_value{};

		[[nodiscard]] auto close() const noexcept
			-> error_code 
		{
			/*
			if this method does not return the error_code
			caller does not know id closing action
			has uscceeded, and that is the whole point

			logic: if handler is handling
			something go ahead and try and close
			return the error_code made in the close()
			*/
			if (*this)
			{
				return handle_trait::close(m_value);
			}
			/*
			otherwise just return ok 
			note: we return enum value
			this will construct the 
			std::error_code	return value
			because we have correctly implemented
			our std::error_code framework
			*/
			return dbj_err_code::sqlite_ok;
		}

	}; // unique_handle

#pragma region connection and statement traits
	struct connection_handle_traits final
	{
		using pointer = sqlite::sqlite3 * ;

		static auto invalid() noexcept
		{
			return nullptr;
		}

		[[nodiscard]] static auto close(pointer value) noexcept 
			-> std::error_code
		{
			return sqlite_ec( sqlite::sqlite3_close(value), DBJ_ERR_PROMPT("sqlite3_close() has failed"));
		}
	};

	using connection_handle = unique_handle<connection_handle_traits>;

	struct statement_handle_traits final
	{
		using pointer = sqlite::sqlite3_stmt * ;

		static auto invalid() noexcept
		{
			return nullptr;
		}

		[[nodiscard]] static auto close(pointer value) noexcept -> std::error_code
		{
			return sqlite_ec(sqlite::sqlite3_finalize(value), DBJ_ERR_PROMPT("sqlite3_finalize() has failed"));
		}
	};

	using statement_handle = unique_handle<statement_handle_traits>;
#pragma endregion 
	

	/*
	return the typed value from a single cell in a column
	for the *current* row from the &current* result set
	also provide the colum names for the same row
	http://www.sqlite.org/c3ref/column_blob.html
	TODO: https://sqlite.org/c3ref/errcode.html 

	 if testing shows necessary we will provide error checking or safe type casts etc. 
	 curently BLOB's are unhandled, they are to be implemented as vector<unsigned char> 
	*/
	struct row_descriptor final
	{
		/*
		sqlite3 holds the values as unsigned char's
		transform to the required C++ type
		for the row from the result set
		*/
		struct transformer final 
		{
			/* if user needs  float, the user will handle that best */
			operator double() const noexcept {
				return sqlite::sqlite3_column_double(statement_, col_index_);
			}
			operator long() const noexcept {
				return sqlite::sqlite3_column_int(statement_, col_index_);
			}
			operator long long() const noexcept  {
				return sqlite::sqlite3_column_int64(statement_, col_index_);
			}
			/* 
			   SQLITE is by default UTF-8
			   wstring is primarily windows curiosity
			   multibyte chars even more so
			   windows users will use this and perform the 
			   required conversions themselves best
			   for pitfalls see here
			   https://docs.microsoft.com/en-us/windows/desktop/api/stringapiset/nf-stringapiset-widechartomultibyte
			*/
			operator dbj::buf::yanb () const noexcept  {
				const unsigned char *name = sqlite::sqlite3_column_text(statement_, col_index_);
				const size_t  sze_ = sqlite::sqlite3_column_bytes(statement_, col_index_);
				_ASSERTE(name);
				return { (char *)name };
			}

			mutable sqlite::sqlite3_stmt *	statement_;
			mutable int			col_index_;
		};

		/* 
		return the transformer for particular column
		from the current result set

		NOTE: sqlite3 simply returns 0 if column index 
		      given is out of range
			  in DEBUG builds we assert on bad index given
			  in release builds we also return 0
		*/
		transformer operator ()(int col_idx ) const noexcept
		{
			_ASSERTE(this->statement_);
			_ASSERTE(!(col_idx > column_count()));
			return transformer{
				this->statement_ , col_idx 
			};
		}
		// return the column name
		// or 0 in index is out of range
		// in release builds
		const char * name(int col_idx) const noexcept
		{
			_ASSERTE(this->statement_);
			_ASSERTE(! (col_idx > column_count()) );
			return sqlite::sqlite3_column_name(
				this->statement_ , col_idx
			);
		}
		// --------------------------------------
		mutable sqlite::sqlite3_stmt * statement_{};

		int column_count() const noexcept {
			_ASSERTE(this->statement_);
			static int count_ = sqlite::sqlite3_column_count(statement_);
			return count_;
		}
	}; // row_descriptor

	// user created callback type for database query()
    // return the sqlite err rezult code or SQLITE_OK
	// called once per each row of the result set
	// this is C++ version of the callback
	// not sqlite3 C version 
	using result_row_callback = int(*)
		(
        const size_t /* the row id */ ,
		const row_descriptor &
		);

	/*
	main interface to the whole dbj++sql
	*/
	class database final
	{
		mutable connection_handle handle{};
/*
this function by design does not return a value 
so we do not return a pair, just the error_code
*/
	[[nodiscard]] static error_code
		dbj_sqlite_open(connection_handle & handle_, char const * filename)	noexcept
	{
		error_code ec;
		handle_.reset(ec);
		// on error return, do not open the db
		if (ec) return ec;
		// make the error_code, log if not OK and return it
		return sqlite_ec(sqlite::sqlite3_open(filename,
			handle_.get_address_of())
		, DBJ_ERR_PROMPT("sqlite3_open has failed."));
	}	
	
	[[nodiscard]] error_code
		prepare_statement (char const * query_, statement_handle & statement_ ) const noexcept
	{
		_ASSERTE(query_);
		if (!handle)
			return to_std_error_code(
				::std::errc::protocol_error, 
				"dbj::db::database -- Must call open() before " __FUNCSIG__ 
			);
		// make the error_code, log if error and return it
		return sqlite_ec(sqlite::sqlite3_prepare_v2(
			handle.get(),
			query_,
			-1,
			statement_.get_address_of(),
			NULL ),
			DBJ_ERR_PROMPT("sqlite3_prepare_v2() has failed"));
	}

	public:
		/* default constructor is non existent */
		database() = delete;
		/* copying and moving is not possible */
		database( const database &) = delete;
		database( database &&) = delete;

		// can *not* throw from this constructor
		explicit database( string_view storage_name, error_code & ec ) noexcept
		{
			ec.clear();
			ec = dbj_sqlite_open( this->handle, storage_name.data() );
		}

		/*
		will do 'something like':
		sqlite::sqlite3_create_function(db, "palindrome", 1, SQLITE_UTF8, NULL, &palindrome, NULL, NULL);
		*/
		[[nodiscard]] error_code register_user_defined_function 
		( 
			string_view udf_name, 
			void(__cdecl * udf_)(sqlite::sqlite3_context *, int, sqlite::sqlite3_value **)
		) const noexcept
		{
			if (!handle)
				return to_std_error_code(
					::std::errc::protocol_error,
					"dbj::db::database -- Must call open() before " __FUNCSIG__
				);

			
			auto const result 
				= sqlite::sqlite3_create_function(
					handle.get(), 
					udf_name.data(), 
					1, 
					SQLITE_UTF8, 
					NULL, /* arbitrary pointer. UDF can gain access using sqlite::sqlite3_user_data().*/
					udf_, 
					NULL, 
					NULL);

			return sqlite_ec(result, DBJ_ERR_PROMPT("sqlite3_create_function() has failed"));
		}

/*
call with a query and a callback, return std::error_code
no error is SQLITE_DONE or SQLITE_OK
*/
[[nodiscard]] error_code query (
	char const * query_, 
	result_row_callback  row_user_ 
) const noexcept
{
		if (!handle)
			return to_std_error_code(
				::std::errc::protocol_error,
				"dbj::db::database -- Must call open() before " __FUNCSIG__
			);
		// will release the statement upon exit
		statement_handle statement_;
		// return if error_code is not 0 
		if ( error_code e = prepare_statement(query_, statement_ );	e ) return e;

#ifdef _DEBUG
		auto col_count_ [[maybe_unused]]= sqlite::sqlite3_column_count(statement_.get());
#endif

	int sql_result ;
		size_t row_counter{};
		while ((sql_result = sqlite::sqlite3_step(statement_.get())) == SQLITE_ROW) {
			// call the callback once per row returned
			sql_result = (row_user_)(
				row_counter++,
				// col_names_,
				{ statement_.get() }
			);
			// break if required
			if (sql_result != SQLITE_OK) break;
	    }
		// if SQLITE_DONE return SQLITE_OK
		if (sql_result == (int)dbj_err_code::sqlite_done )
			return dbj_err_code::sqlite_ok;
	    return sqlite_ec(sql_result, DBJ_ERR_PROMPT("sqlite3_step() has failed"));
	}
/*
 execute SQL statements through here for which no result set is expected
*/
		[[nodiscard]] error_code exec( const char * sql_ ) const noexcept
		{
			_ASSERTE(sql_);
			if (!handle)
				return to_std_error_code(
					::std::errc::protocol_error,
					"dbj::db::database -- Must call open() before " __FUNCSIG__
				);

			int sql_result = sqlite::sqlite3_exec(
				handle.get(), /* An open database */
				sql_,		/* SQL to be evaluated */
				nullptr,	/* Callback function */
				nullptr,	/* 1st argument to callback */
				nullptr		/* Error msg written here */
			);

			// this will log if sql_result was error
		std::error_code ec_
				= sqlite_ec(sql_result, DBJ_ERR_PROMPT("sqlite3_exec() failed"));

			return ec_ ;
		}

	}; // database

#pragma region dbj sqlite easy udf

	struct udf_argument final
	{
		/* curently BLOB's are unhandled, they are to be implemented as vector<unsigned char> */
		struct transformer final
		{
			/* if user needs  float, the user will handle that best */
			operator double() const noexcept {
				if (sqlite::sqlite3_value_type(argv[col_index_]) != SQLITE_FLOAT)
				{
				}
				return sqlite::sqlite3_value_double(argv[col_index_]);
			}
			operator long() const noexcept {
				if (sqlite::sqlite3_value_type(argv[col_index_]) != SQLITE_INTEGER)
				{
				}
				return sqlite::sqlite3_value_int(argv[col_index_]);
			}
			operator long long() const noexcept
			{
				if (sqlite::sqlite3_value_type(argv[col_index_]) != SQLITE_INTEGER)
				{
				}
				return sqlite::sqlite3_value_int64(argv[col_index_]);
			}
			// NOTE: we do not use std::string. It is slow and big, but 
			// good at what it is mean for and that is not to be a char buffer
			// dbj::buf::yanb == yet another buffer
			operator dbj::buf::yanb () const noexcept
			{
				if (sqlite::sqlite3_value_type(argv[col_index_]) != SQLITE_TEXT) {
				}
				char *text = (char*)sqlite::sqlite3_value_text(argv[col_index_]);
				_ASSERTE(text);
				size_t text_length = sqlite::sqlite3_value_bytes(argv[col_index_]);
				_ASSERTE(text_length > 0);
				return { text };
			}

			mutable sqlite::sqlite3_value **argv{};
			mutable size_t			 col_index_;
		};

		/* number of arguments */
		size_t arg_count() const noexcept { return this->argc_; }

		/*	return the transformer for a column	*/
		udf_argument::transformer
			operator ()(size_t col_idx) const 
		{
			_ASSERTE(this->argv_);

			if (col_idx > argc_)
				// throw error_instance(error_code::UDF_ARGC_INVALID);
			throw std::make_error_code( errc::result_out_of_range );

			return transformer{
				this->argv_, col_idx
			};
		}
		// --------------------------------------
		mutable sqlite::sqlite3_value	**argv_{};
		mutable size_t			argc_;
	}; // udf_argument

	struct udf_retval final {

		mutable sqlite::sqlite3_context *context_;

		// this will cause the sqlite3 to throw the exeception from
		// the udf using mechanism
		void return_error(const std::string & msg_) const noexcept
		{
			sqlite::sqlite3_result_error(context_, msg_.c_str(), 
				static_cast<int>(msg_.size()));
		}

		// sink the result using the appropriate sqlite3 function
		void  operator () (const std::string & value_) const noexcept
		{
			sqlite::sqlite3_result_text(context_, value_.c_str(), 
				static_cast<int>(value_.size()), nullptr);
		}

		void  operator () (std::string_view value_) const noexcept
		{
			sqlite::sqlite3_result_text(context_, value_.data(), 
				static_cast<int>(value_.size()), nullptr);
		}

		void operator () (double value_) const noexcept
		{
			sqlite::sqlite3_result_double(context_, value_);
		}

		void operator () (int value_) const noexcept
		{
			sqlite::sqlite3_result_int(context_, value_);
		}

		void operator () (long value_) const noexcept
		{
			sqlite::sqlite3_result_int(context_, value_);
		}

		void operator () (long long value_) const noexcept
		{
			sqlite::sqlite3_result_int64(context_, value_);
		}

		void operator () (nullptr_t) const noexcept
		{
			sqlite::sqlite3_result_null(context_);
		}
	}; // udf_retval

using dbj_sql_udf_type =
	void(*) (const udf_argument  &, const udf_retval &);

using sqlite3_udf_type = void(__cdecl *)
(sqlite::sqlite3_context *context, int argc, sqlite::sqlite3_value **argv);

template<dbj_sql_udf_type udf_>
struct udf_holder final
{
static void function
(sqlite::sqlite3_context *context,
	int argc,
	sqlite::sqlite3_value **argv)
{
	(void)noexcept(argc); // unused for now
	_ASSERTE(context);
	_ASSERTE(argv);
	udf_argument  values_{ argv, size_t(argc) };
	udf_retval result_{ context };
	udf_(values_, result_);
}
}; // udf_holder

	template<dbj_sql_udf_type dbj_udf_>
	[[nodiscard]] inline std::error_code 
	register_dbj_udf(
		database const & db,
		char const * dbj_udf_name_
	) noexcept
	{
		_ASSERTE(dbj_udf_name_);
		using udf_container_type = udf_holder<dbj_udf_>;
		sqlite3_udf_type udf_ = &udf_container_type::function;
		std::error_code ec =  db.register_user_defined_function(dbj_udf_name_, udf_);
		return ec;
	}

#pragma endregion dbj sqlite easy udf

#pragma region various utilities
/*
https://stackoverflow.com/questions/1601151/how-do-i-check-in-sqlite-whether-a-table-exists?rq=1

PRAGMA table_info(your_table_name)
If the resulting table is empty then your_table_name doesn't exist.

Documentation:

PRAGMA schema.table_info(table-name);

This pragma returns one row for each column in the named table. Columns in the result set 
include the column name, data type, whether or not the column can be NULL, and the default 
value for the column. The "pk" column in the result set is zero for columns that are not 
part of the primary key, and is the index of the column in the primary key for columns that 
are part of the primary key.

The table named in the table_info pragma can also be a view.

Example output:

cid|name|type|notnull|dflt_value|pk
0|id|INTEGER|0||1
1|json|JSON|0||0
2|name|TEXT|0||0
*/
	[[nodiscard]] inline error_code 
		table_info(database const & db, 
			std::string_view table_name ,
			result_row_callback  result_callback_ )
	noexcept
	{
		using ::dbj::fmt::to_buff;

		auto qry = to_buff("PRAGMA table_info('%s')", table_name);

		// list of all tables and views
		// auto qry = "SELECT name, sql FROM sqlite_master WHERE type = 'table' ORDER BY name;"sv;

		return db.query(qry.data(), result_callback_ );
	}
#pragma endregion
} // namespace dbj::db

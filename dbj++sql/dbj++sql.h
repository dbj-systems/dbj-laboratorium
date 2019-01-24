#pragma once

#include "sqlite++.h"
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <cstdio>
#include <crtdbg.h>
#include "./err/dbj_db_err.h"
#include "./dbj_log/dbj_log_user.h"

#ifndef DBJ_STR
#define DBJ_STR(x) #x
#endif

#ifndef DBJ_VANISH
#define DBJ_VANISH(...) static_assert( (noexcept(__VA_ARGS__),true) );
#endif
#ifndef DBJ_VERIFY
#define DBJ_VERIFY_(x, file, line ) if (false == x ) ::dbj::db::terror( #x ", failed", file, line )
#define DBJ_VERIFY(x) DBJ_VERIFY_(x,__FILE__,__LINE__)
#endif


namespace dbj::db {

	using namespace ::std;
	// using namespace ::sqlite;
	using namespace ::std::string_view_literals;

	using namespace ::dbj::db::err;

[[noreturn]] inline void terror
(const char * msg_, const char * file_, const int line_)
{
	_ASSERTE(msg_);	_ASSERTE(file_);_ASSERTE(line_);
	::fprintf(stderr, "\n\ndbj++sql Terminating error:%s\n%s (%d)", msg_, file_, line_);
	exit(EXIT_FAILURE);
}
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
	so it is logicaly resilient
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
			// must not throw exceptions
			// from destructors
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

		auto get_address_of() noexcept -> pointer *
		{
			_ASSERTE(!*this);
			return &m_value;
		}

		auto release() noexcept -> pointer
		{
			auto value = m_value;
			m_value = handle_trait::invalid();
			return value;
		}

		auto reset(pointer value = handle_trait::invalid()) -> bool
		{
			if (m_value != value)
			{
				// the only reasonable course
				// of action here, is
				// to throw the error code
				// if handle can not be closed 
				// there is no point of using it
				if (auto ec = close(); !is_sql_err_ok(ec)) throw ec;

				m_value = value;
			}
			// API designer wants to return a bool 
			// thus we had to throw the not ok error_code
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
			*/
			return dbj_err_code::sqlite_ok;
		}

	}; // unique_handle

#pragma region connection and statement traits
	struct connection_handle_traits final
	{
		using pointer = sqlite::sqlite3 * ;

		/*
		be carefull with copy() protocol
		think twice if you can copy, how to copy etc.
		*/
		static pointer copy( pointer p_ ) noexcept {
			// in this context this is ok
			return p_;
		}

		static auto invalid() noexcept
		{
			return nullptr;
		}

		[[nodiscard]] static auto close(pointer value) noexcept -> std::error_code
		{
			return sqlite_ec( sqlite::sqlite3_close(value) );
		}
	};

	using connection_handle = unique_handle<connection_handle_traits>;

	struct statement_handle_traits final
	{
		using pointer = sqlite::sqlite3_stmt * ;

		/*
		be carefull with copy() protocol
		think twice when you can copy, how to copy etc.
		*/
		static pointer copy(pointer p_) noexcept {
			// in this case this is ok
			return p_;
		}

		static auto invalid() noexcept
		{
			return nullptr;
		}

		[[nodiscard]] static auto close(pointer value) noexcept -> std::error_code
		{
			return sqlite_ec(sqlite::sqlite3_finalize(value));
		}
	};

	using statement_handle = unique_handle<statement_handle_traits>;
#pragma endregion 
	

	/*
	return the typed value from a single cell in a column
	for the *current* row of the result
	*/
	struct row_descriptor final
	{
		/* http://www.sqlite.org/c3ref/column_blob.html */
	   /* TODO: https://sqlite.org/c3ref/errcode.html  */

		/* if testing shows necessary we will provide error checking or safe type casts etc.  */
		/* curently BLOB's are unhandled, they are to be implemented as vector<unsigned char> */
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
			operator std::string() const noexcept  {
				const unsigned char *name = sqlite::sqlite3_column_text(statement_, col_index_);
				const size_t  sze_ = sqlite::sqlite3_column_bytes(statement_, col_index_);
				return { (const char *)name, sze_ };
			}

			mutable sqlite::sqlite3_stmt *	statement_;
			mutable int			col_index_;
		};

		/* 
		return the transformer for particular column
		from the current result set
		*/
		transformer operator ()(int col_idx ) const noexcept
		{
			_ASSERTE(this->statement_);
			_ASSERTE(!(col_idx > column_count()));
			return transformer{
				this->statement_ , col_idx 
			};
		}
		// the column name
		const char * name(int col_idx) const noexcept
		{
			_ASSERTE(this->statement_);
			_ASSERTE(! (col_idx > column_count()) );
			return sqlite3_column_name(
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
	// called once per the row of the result set
	using result_row_user_type = int(*)
		(
        const size_t /* result row id */ ,
		const row_descriptor &
		);

	/*
	main interface 
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
		handle_.reset();
		auto const result = sqlite::sqlite3_open(filename,
			handle_.get_address_of());

		return sqlite_ec( result );
	}	
	
	[[nodiscard]] error_code
		prepare_statement (char const * query_, statement_handle & statement_ ) const noexcept
	{
		_ASSERTE(query_);
		if (!handle)
			return std_ec(
				::std::errc::protocol_error, 
				"dbj::db::database -- Must call open() before " __FUNCSIG__ 
			);
		auto const result = sqlite::sqlite3_prepare_v2(
			handle.get(),
			query_,
			-1,
			statement_.get_address_of(),
			NULL );

			return sqlite_ec(result); 
			//usualy SQLITE_OK as an error_code 
	}

	public:
		/* default constructor is non existent */
		database() = delete;
		/* copying and moving is not possible */
		database( const database &) = delete;
		database( database &&) = delete;

		// can *not* throw
		explicit database( string_view storage_name, error_code & ec ) noexcept
		{
			ec.clear();
			// not returning pair, by design
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
				return std_ec(
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

			return sqlite_ec(result);
		}

/*
call with query and a callback
return std::error_code
*/
[[nodiscard]] error_code query (
	char const * query_, 
	result_row_user_type  row_user_ 
) const noexcept
{
		if (!handle)
			return std_ec(
				::std::errc::protocol_error,
				"dbj::db::database -- Must call open() before " __FUNCSIG__
			);

		statement_handle statement_;
		// return if error_code is not 0 
		if ( error_code e = prepare_statement(query_, statement_ );	e ) return e;

	int sql_result ;
		size_t row_counter{};
		while ((sql_result = sqlite::sqlite3_step(statement_.get())) == SQLITE_ROW) {
			// call once per row returned
			sql_result = (row_user_)(
				row_counter++,
				// col_names_,
				{ statement_.get() }
			);
	    }
		// users to check for SQLITE_DONE;
	    return sqlite_ec(sql_result );
	}
//-------------------------------------------------------------------------------
		[[nodiscard]] error_code exec( const char * sql_ ) const noexcept
		{
			_ASSERTE(sql_);
			if (!handle)
				return std_ec(
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

			// check for SQLITE_OK;
			return sqlite_ec(sql_result);
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
			operator std::string() const noexcept
			{
				if (sqlite::sqlite3_value_type(argv[col_index_]) != SQLITE_TEXT) {
				}
				char *text = (char*)sqlite::sqlite3_value_text(argv[col_index_]);
				_ASSERTE(text);
				size_t text_length = sqlite::sqlite3_value_bytes(argv[col_index_]);
				_ASSERTE(text_length > 0);
				return { text, text_length };
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
};

	template<dbj_sql_udf_type dbj_udf_>
	inline void register_dbj_udf(
		const dbj::db::database & db,
		const char * dbj_udf_name_
	)
	{
		_ASSERTE(dbj_udf_name_);
		using udf_container_type = udf_holder<dbj_udf_>;
		sqlite3_udf_type udf_ = &udf_container_type::function;
		std::error_code ec =  db.register_user_defined_function(dbj_udf_name_, udf_);

		if (!is_sql_err_ok(ec))
			throw ec;
	};

#pragma endregion dbj sqlite easy udf

} // namespace dbj::db

#undef DBJ_VERIFY_
#undef DBJ_STR
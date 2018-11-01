#pragma once
//  
// (very) small SQLITE standard C++  framework, dbj.org created
// 
// inspired by:
// https://visualstudiomagazine.com/articles/2014/02/01/using-sqlite-with-modern-c.aspx
//
// requires C++17
// #define DBJ_DB_TESTING for testing, see the file bottom part
//
#include "sqlite/sqlite3.h"
#include <string>
#include <optional>
#include <vector>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <crtdbg.h>

namespace dbj::sqlite {

	using namespace std;

	struct sql_exception
	{
		using message_arg_type = char const *;
		int code;
		string message;

		sql_exception(int const result, message_arg_type text) :
			code{ result },
			message{ text }
		{}
	};
}

#ifndef DBJ_STR
#define DBJ_STR(x) #x
#endif

#ifndef DBJ_VERIFY_
#define DBJ_VERIFY_(R,X) if ( R != X ) \
   throw dbj::sqlite::sql_exception( int(R), __FILE__ "(" DBJ_STR(__LINE__) ")\n" DBJ_STR(R) " != " DBJ_STR(X))
#endif
//NOTE: leave it here
#include "handle.h"

namespace dbj::sqlite {

	using namespace KennyKerr;
	using namespace std;

	struct connection_handle_traits final
	{
		using pointer = sqlite3 * ;

		static auto invalid() noexcept
		{
			return nullptr;
		}

		static auto close(pointer value) 
		{
			DBJ_VERIFY_(SQLITE_OK, sqlite3_close(value));
		}
	};

	using connection_handle = unique_handle<connection_handle_traits>;

	struct statement_handle_traits final
	{
		using pointer = sqlite3_stmt * ;

		static auto invalid() noexcept
		{
			return nullptr;
		}

		static auto close(pointer value) 
		{
			DBJ_VERIFY_(SQLITE_OK, sqlite3_finalize(value));
		}
	};

	using statement_handle = unique_handle<statement_handle_traits>;

	// DBJ added
	using callback_type = 
		int (*)(void * /* a_param */, int /* argc */, char ** /* argv */, char ** /* column */);
	// above runs once for each line returned

	/*
	return the typed value from a single cell in a column
	for the *current* row of the result
	*/
	struct value_decoder final
	{
		/* http://www.sqlite.org/c3ref/column_blob.html */
	   /* TODO: https://sqlite.org/c3ref/errcode.html  */

		/* if testing shows necessary we will provide error checking or safe type casts etc.  */
		/* curently BLOB's are unhandled, they are to be implemented as vector<unsigned char> */
		struct transformer final 
		{
			/* if user needs  float, the user will handle that best */
			operator double() const noexcept {
				return sqlite3_column_double(statement_, col_index_);
			}
			operator long() const noexcept {
				return sqlite3_column_int(statement_, col_index_);
			}
			operator long long() const noexcept  {
				return sqlite3_column_int64(statement_, col_index_);
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
				const unsigned char *name = sqlite3_column_text(statement_, col_index_);
				const size_t  sze_ = sqlite3_column_bytes(statement_, col_index_);
				return { (const char *)name, sze_ };
			}

			mutable sqlite3_stmt *	statement_;
			mutable size_t			col_index_;
		};

		/* 
		return the transformer for particular column
		from the current result set
		*/
		transformer operator ()(size_t col_idx ) const noexcept
		{
			_ASSERTE(this->statement_);
			return transformer{
				this->statement_ ,col_idx 
			};
		}
		// --------------------------------------
		mutable sqlite3_stmt * statement_{};
	}; // value_decoder

	/*
	return vector of column names from the active statement
	*/
	inline vector<string> column_names( const statement_handle & sh_ ) 
	{
		// this actually calls the traits invalid method
		// from inside the handle bool operator
		_ASSERTE( sh_ );

		const size_t column_count = sqlite3_column_count(sh_.get());
		vector<string> names{};
		for (size_t n = 0; n < column_count; ++n) {
			names.push_back( { sqlite3_column_name(sh_.get(), n) } );
		}
		return names;
	}

	// user created callback type for database execute_with_statement()
    // return the sqlite rezult code or SQLITE_OK
	// called once per the row of the result set
	using result_row_user_type = int(*)
		(
        const size_t /* result row id */ ,
		const vector<string> & /*column_names*/,
		const value_decoder &
		);

	/*
	main interface 
	*/
	class database final
	{
		connection_handle handle;

	auto open(char const * filename)
	{
		auto local = connection_handle{};

		auto const result = sqlite3_open(filename,
			local.get_address_of());

		if (SQLITE_OK != result)
		{
			throw sql_exception{ result, sqlite3_errmsg(local.get()) };
		}

		handle = move(local);
	}	
	
	statement_handle prepare_statement (char const * query_)
	{
		_ASSERTE(query_);
		if (!handle) throw dbj::sqlite::sql_exception(0, " Must call open() before " __FUNCSIG__);

		auto local_statement = statement_handle{};

		auto const result = sqlite3_prepare_v2(
			handle.get(),
			query_,
			-1,
			local_statement.get_address_of(),
			NULL );

		if (SQLITE_OK != result)
		{
			throw sql_exception{ result, sqlite3_errmsg(handle.get()) };
		}

		// instead of: this->statement_ = move(local_statement);
		return local_statement ;
	}

	public:
		/* default constructor is non existent */
		database() = delete;
		/* copying and moving is no possible */
		database( const database &) = delete;
		database( database &&) = delete;

		explicit database( string_view storage_name ) 
		{
			// can throw
			open(storage_name.data() );
		}

		/*
		will do 'something like':
		sqlite3_create_function(db, "palindrome", 1, SQLITE_UTF8, NULL, &palindrome, NULL, NULL);
		*/
		auto register_user_defined_function
		( 
			string_view udf_name, 
			void(__cdecl * udf_)(sqlite3_context *, int, sqlite3_value **)
		) 
		{
			if (!handle) throw dbj::sqlite::sql_exception(0, " Must call open() before " __FUNCSIG__);
			auto const result 
				= sqlite3_create_function(
					handle.get(), 
					udf_name.data(), 
					1, 
					SQLITE_UTF8, 
					NULL, /* arbitrary pointer. can gain access using sqlite3_user_data().*/
					udf_, 
					NULL, 
					NULL);

			if (SQLITE_OK != result)
			{
				throw sql_exception{ result, sqlite3_errmsg(handle.get()) };
			}
		}

	auto execute
	(
		char const * query_,
		optional<callback_type> the_callback = nullopt
	)
	{
		if (!handle) throw dbj::sqlite::sql_exception(0, " Must call open() before " __FUNCSIG__);

		auto const result = sqlite3_exec(
			handle.get(), /* the db */
			query_,
			the_callback.value_or(nullptr) ,
			nullptr, /* first callback void * param, passed through here */
			nullptr);

		if (SQLITE_OK != result)
		{
			throw sql_exception{ result, sqlite3_errmsg(handle.get()) };
		}
	}	
/*
https://stackoverflow.com/questions/31146713/sqlite3-exec-callback-function-clarification#
*/
	auto execute_with_statement (
		char const * query_, 
		optional<result_row_user_type>  statement_user_arg_
	)
	{
		if ( !statement_user_arg_.has_value() )
			throw dbj::sqlite::sql_exception(0, " statement_user_ argument must exist for  " __FUNCSIG__);
		if (!handle) 
			throw dbj::sqlite::sql_exception(0, " Must call open() before " __FUNCSIG__);

		statement_handle statement_ = prepare_statement(query_);
		result_row_user_type statement_user_ = statement_user_arg_.value(); 
		vector<string> col_names_ { column_names( statement_ ) };

		int rc{};
		size_t row_counter{};
		while ((rc = sqlite3_step(statement_.get())) == SQLITE_ROW) {
			// call once per row returned
			rc = statement_user_(
				row_counter ++ ,
				col_names_ ,
				{ statement_.get() }
			);
			// CAUTION! no other kind of exception caught 
			// *possibly* coming out of statement_user_
		}

		if (rc != SQLITE_DONE) {
			throw dbj::sqlite::sql_exception(rc, sqlite3_errmsg(handle.get()));
		}
	}

	}; // database

	/*
	-------------------------------------------------------------------------
		TESTS
	-------------------------------------------------------------------------
	*/   
#ifdef DBJ_DB_TESTING

	inline  auto test_insert (const char * db_file = ":memory:")
	{
		try
		{
			database c(db_file);
			// we do not send a callback in the calls bellow
			c.execute("DROP TABLE IF EXISTS Hens");
			c.execute("CREATE TABLE Hens ( Id int primary key, Name nvarchar(100) not null )");
			c.execute("INSERT INTO Hens (Id, Name) values (1, 'Rowena'), (2, 'Henrietta'), (3, 'Constance')");
			c.execute("SELECT Name FROM Hens WHERE Name LIKE 'Rowena'");
			// p.s. Above is Kenny Kerrs humour table :)
		}
		catch (sql_exception const & e)
		{
			wprintf(L"dbj::sqlite exception");
			wprintf(L"%d %S\n", e.code, e.message.c_str());
		}
	}

	inline  auto test_select ( 
		callback_type cb_,
	    const char * db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"
	)
	{
		try
		{
			database c(db_file);
			c.execute("select word from words where word like 'bb%'", cb_ );
		}
		catch (sql_exception const & e)
		{
			wprintf(L"dbj::sqlite exception");
			wprintf(L"%d %S\n", e.code, e.message.c_str());
		}
	}	
	
	inline  auto test_statement_using ( 
		result_row_user_type row_user_ ,
	    const char * db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"
	)
	{
		try
		{
			database c(db_file);
			c.execute_with_statement("select word from words where word like 'bb%'", 
				row_user_);
		}
		catch (sql_exception const & e)
		{
			wprintf(L"dbj::sqlite exception");
			wprintf(L"%d %S\n", e.code, e.message.c_str());
		}
	}
#endif // DBJ_DB_TESTING

} // namespace dbj::sqlite

#undef DBJ_VERIFY_
#undef DBJ_STR
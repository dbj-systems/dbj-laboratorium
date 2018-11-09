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
#include "sqlite++.h"
#include <string>
#include <optional>
#include <vector>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <crtdbg.h>

namespace dbj::db {

	using namespace ::std;
	using namespace ::sqlite;

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

enum class DBJ_DB_ERROR { OK = -1, ERR = -2 };

#ifndef DBJ_STR
#define DBJ_STR(x) #x
#endif

#ifndef DBJ_VERIFY_
#define DBJ_VERIFY_(R,X) if ( R != X ) \
   throw ::dbj::db::sql_exception( int(R), __FILE__ "(" DBJ_STR(__LINE__) ")\n" DBJ_STR(R) " != " DBJ_STR(X))
#endif
//NOTE: leave it here
#include "handle.h"

namespace dbj::db {

	using namespace ::KennyKerr;
	using namespace ::std;
	using namespace ::sqlite;

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

	// user created callback type for database query()
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
		mutable connection_handle handle;

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
	
	statement_handle prepare_statement (char const * query_) const
	{
		_ASSERTE(query_);
		if (!handle) throw dbj::db::sql_exception(0, " Must call open() before " __FUNCSIG__);

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
		/* copying and moving is not possible */
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
		) const
		{
			if (!handle) throw dbj::db::sql_exception(0, " Must call open() before " __FUNCSIG__);
			auto const result 
				= sqlite3_create_function(
					handle.get(), 
					udf_name.data(), 
					1, 
					SQLITE_UTF8, 
					NULL, /* arbitrary pointer. UDF can gain access using sqlite3_user_data().*/
					udf_, 
					NULL, 
					NULL);

			if (SQLITE_OK != result)
			{
				throw sql_exception{ result, sqlite3_errmsg(handle.get()) };
			}
		}

	/*
	call with query and a callback
	*/
	auto query (
		char const * query_, 
		optional<result_row_user_type>  row_user_ = nullopt
	) const
	{
		if (!handle) 
			throw dbj::db::sql_exception(0, " Must call open() before " __FUNCSIG__);

		statement_handle statement_ = prepare_statement(query_);
		vector<string> col_names_ { column_names( statement_ ) };
		
	int rc{};
		size_t row_counter{};
		while ((rc = sqlite3_step(statement_.get())) == SQLITE_ROW) {
		if (row_user_.has_value()) {
			// call once per row returned
			rc = (row_user_.value())(
				row_counter++,
				col_names_,
				{ statement_.get() }
			);
		  }
	    }

		if (rc != SQLITE_DONE) {
			throw dbj::db::sql_exception(rc, sqlite3_errmsg(handle.get()));
		}
	}

	}; // database

#pragma region dbj sqlite easy udf

	struct dbj_sql_udf_value final
	{
		/* curently BLOB's are unhandled, they are to be implemented as vector<unsigned char> */
		struct transformer final
		{
			/* if user needs  float, the user will handle that best */
			operator double() const noexcept {
				if (sqlite3_value_type(argv[col_index_]) != SQLITE_FLOAT)
				{
				}
				return sqlite3_value_double(argv[col_index_]);
			}
			operator long() const noexcept {
				if (sqlite3_value_type(argv[col_index_]) != SQLITE_INTEGER)
				{
				}
				return sqlite3_value_int(argv[col_index_]);
			}
			operator long long() const noexcept
			{
				if (sqlite3_value_type(argv[col_index_]) != SQLITE_INTEGER)
				{
				}
				return sqlite3_value_int64(argv[col_index_]);
			}
			operator std::string() const noexcept
			{
				if (sqlite3_value_type(argv[col_index_]) != SQLITE_TEXT) {
				}
				char *text = (char*)sqlite3_value_text(argv[col_index_]);
				_ASSERTE(text);
				size_t text_length = sqlite3_value_bytes(argv[col_index_]);
				_ASSERTE(text_length > 0);
				return { text, text_length };
			}

			mutable sqlite3_value **argv{};
			mutable size_t			 col_index_;
		};

		/*	return the transformer for a column	*/
		dbj_sql_udf_value::transformer
			operator ()(size_t col_idx) const noexcept
		{
			_ASSERTE(this->argv_);
			return transformer{
				this->argv_, col_idx
			};
		}
		// --------------------------------------
		mutable sqlite3_value **argv_{};
	}; // dbj_sql_udf_value

	struct dbj_sql_udf_retval final {

		mutable sqlite3_context *context_;

		// this will cause the sqlite3 to throw the exeception from
		// the udf using mechanism
		void return_error(const std::string & msg_) const noexcept
		{
			sqlite3_result_error(context_, msg_.c_str(), msg_.size());
		}

		// sink the result using the appropriate sqlite3 function

		void  operator () (const std::string & value_) const noexcept
		{
			sqlite3_result_text(context_, value_.c_str(), value_.size(), nullptr);
		}

		void  operator () (std::string_view value_) const noexcept
		{
			sqlite3_result_text(context_, value_.data(), value_.size(), nullptr);
		}

		void operator () (double value_) const noexcept
		{
			sqlite3_result_double(context_, value_);
		}

		void operator () (int value_) const noexcept
		{
			sqlite3_result_int(context_, value_);
		}

		void operator () (long value_) const noexcept
		{
			sqlite3_result_int(context_, value_);
		}

		void operator () (long long value_) const noexcept
		{
			sqlite3_result_int64(context_, value_);
		}

		void operator () (nullptr_t) const noexcept
		{
			sqlite3_result_null(context_);
		}
	}; // dbj_sql_udf_retval

	using dbj_sql_udf_type =
		void(*) (const dbj_sql_udf_value  &, const dbj_sql_udf_retval &);


	using sqlite3_udf_type = void(__cdecl *)
		(sqlite3_context *context, int argc, sqlite3_value **argv);

	template<dbj_sql_udf_type udf_>
	struct udf_holder final
	{
		static void function
		(sqlite3_context *context,
			int argc,
			sqlite3_value **argv)
		{
			(void)noexcept(argc); // unused for now
			_ASSERTE(context);
			_ASSERTE(argv);
			dbj_sql_udf_value  values_{ argv };
			dbj_sql_udf_retval result_{ context };
			udf_(values_, result_);
		}
	};

	template<dbj_sql_udf_type dbj_udf_>
	inline void register_dbj_udf(
		const dbj::db::database & db,
		const char * dbj_udf_name_
	)
	{
		using udf_container_type = udf_holder<dbj_udf_>;
		sqlite3_udf_type udf_ = &udf_container_type::function;
		db.register_user_defined_function(dbj_udf_name_, udf_);
	};

#pragma endregion dbj sqlite easy udf

} // namespace dbj::db

	/*
	-------------------------------------------------------------------------
		TESTS
	-------------------------------------------------------------------------
	*/   
#ifdef DBJ_DB_TESTING

namespace dbj_db_test_
{
	using namespace dbj::db;

	inline auto create_demo_db( const database & db)
	{
		db.query("DROP TABLE IF EXISTS demo");
		db.query("CREATE TABLE demo_table ( Id int primary key, word nvarchar(100) not null )");
		db.query("INSERT INTO demo_table (Id, Name) values (1, 'London'), (2, 'Glasgow'), (3, 'Cardif')");
	}

	inline  auto test_insert(const char * db_file = ":memory:")
	{
		try
		{
			database db(db_file);
			create_demo_db(db);
		}
		catch (sql_exception const & e)
		{
			wprintf(L"dbj::db exception");
			wprintf(L"%d %S\n", e.code, e.message.c_str());
		}
	}

	inline  auto test_select(
		result_row_user_type cb_,
		const char * db_file = ":memory:" // "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"
	)
	{
		try
		{
			database db(db_file);
			create_demo_db(db);
			// select from the table
			db.query("SELECT word FROM demo_table WHERE word LIKE 'G%'", cb_ );
		}
		catch (sql_exception const & e)
		{
			wprintf(L"dbj::db exception");
			wprintf(L"%d %S\n", e.code, e.message.c_str());
		}
	}

/*
   As a sample DB, I am using an English dictionary in a file,
   https://github.com/dwyl/english-words/
   which I have transformed in the SQLite 3 DB file.
   It has a single table: words, with a single text column named word.
   this is full path to my SQLIte storage
   please replace it with yours
   for that use one of the many available SQLite management app's
*/
	inline  auto test_statement_using(
		result_row_user_type row_user_,
		const char * db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"
	)
	{
		try
		{
			database db(db_file);
			create_demo_db(db);
			// provoke error
			db.query("select word from words where word like 'bb%'",
				row_user_);
		}
		catch (sql_exception const & e)
		{
			wprintf(L"\ndbj::db exception\n\t[%d] %S\n", e.code, e.message.c_str());
		}
	}
} // nspace
#endif // DBJ_DB_TESTING
#undef DBJ_VERIFY_
#undef DBJ_STR
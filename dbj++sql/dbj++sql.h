#pragma once
//
// https://visualstudiomagazine.com/articles/2014/02/01/using-sqlite-with-modern-c.aspx
//
#include <string>
#include <optional>
#include "handle.h"
#include "sqlite/sqlite3.h"

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

	struct value_decoder final
	{
		/* http://www.sqlite.org/c3ref/column_blob.html */
	   /* TODO: https://sqlite.org/c3ref/errcode.html  */

		struct transformer final 
		{
			operator double() const noexcept {
				return sqlite3_column_double(statement_, col_index_);
			}
			operator long() const noexcept {
				return sqlite3_column_int(statement_, col_index_);
			}
			operator long long() const noexcept  {
				return sqlite3_column_int64(statement_, col_index_);
			}
			operator std::string() const noexcept  {
				const unsigned char *name = sqlite3_column_text(statement_, col_index_);
				const size_t  sze_ = sqlite3_column_bytes(statement_, col_index_);
				return { (const char *)name, sze_ };
			}
			operator std::wstring() const noexcept  {
				const unsigned char *name = sqlite3_column_text(statement_, col_index_);
				const size_t  sze_ = sqlite3_column_bytes(statement_, col_index_);
				std::string local_{ (const char *)name, sze_ };
				return { local_.begin(), local_.end() };
			}

			mutable sqlite3_stmt *	statement_;
			mutable size_t			col_index_;
		};

		transformer operator ()(size_t col_idx = 0) const noexcept
		{
			_ASSERTE(this->statement_);
			return transformer{
				this->statement_ ,
				(col_idx ? col_idx : this->col_index_)
			};
		}
		// --------------------------------------
		mutable sqlite3_stmt * statement_{};
		mutable size_t col_index_{};
	}; // value_decoder

	// use as argument type for connection execute_with_statement()
	// return the sqlite rezult code or SQLITE_OK
	using statement_user_type = int(*)( const value_decoder &);

	struct connection final
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

	auto execute
	(
		char const * query_,
		callback_type the_callback 
	)
	{
		if (!handle) throw dbj::sqlite::sql_exception(0, " Must call open() before " __FUNCSIG__);

		statement_handle statement_ = prepare_statement(query_);
		value_decoder    decoder_{ statement_.get(), 0 };
		auto const result = sqlite3_exec(
			handle.get(), /* the db */
			query_,
			the_callback,
			&(decoder_), /* a_param passed here */
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
		optional<statement_user_type>  statement_user_arg_
	)
	{
		if ( !statement_user_arg_.has_value() )
			throw dbj::sqlite::sql_exception(0, " statement_user_ argument must exist for  " __FUNCSIG__);
		if (!handle) 
			throw dbj::sqlite::sql_exception(0, " Must call open() before " __FUNCSIG__);

		statement_handle statement_ = prepare_statement(query_);

		statement_user_type statement_user_ = statement_user_arg_.value();  

		int rc{};
		size_t col_counter{};
		while ((rc = sqlite3_step(statement_.get())) == SQLITE_ROW) {
			// call once per row returned
			rc = statement_user_(
				value_decoder{
					statement_.get(), col_counter
				}
			);
			// CAUTION! no other kind of exception caught 
			// possibly coming out of statement_user_
		}

		if (rc != SQLITE_DONE) {
			throw dbj::sqlite::sql_exception(rc, sqlite3_errmsg(handle.get()));
		}
	}

	}; // execute_with_statement

	/*
	-------------------------------------------------------------------------
		TESTS
	-------------------------------------------------------------------------
	*/
	inline  auto test_insert (const char * db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db")
	{
		try
		{
			connection c;
			c.open(db_file);
			c.execute("DROP TABLE IF EXISTS Hens");
			c.execute("CREATE TABLE Hens ( Id int primary key, Name nvarchar(100) not null )");
			c.execute("INSERT INTO Hens (Id, Name) values (1, 'Rowena'), (2, 'Henrietta'), (3, 'Constance')");
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
			connection c;
			c.open(db_file);
			c.execute("select word from words where word like 'bb%'", cb_ );
		}
		catch (sql_exception const & e)
		{
			wprintf(L"dbj::sqlite exception");
			wprintf(L"%d %S\n", e.code, e.message.c_str());
		}
	}	
	
	inline  auto test_statement_using ( 
		statement_user_type statement_user_ ,
	    const char * db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"
	)
	{
		try
		{
			connection c;
			c.open(db_file);
			c.execute_with_statement("select word from words where word like 'bb%'", 
				statement_user_);
		}
		catch (sql_exception const & e)
		{
			wprintf(L"dbj::sqlite exception");
			wprintf(L"%d %S\n", e.code, e.message.c_str());
		}
	}

} // namespace dbj::sqlite
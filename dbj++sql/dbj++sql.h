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
	using callback_type = int (*)(void * /* a_param */, int /* argc */, char ** /* argv */, char ** /* column */);
	// above runs once for each line returned
	//

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

	auto execute(char const * text, 
		optional<callback_type>  the_callback = nullptr 
	)
	{
		_ASSERTE(handle);

		auto const result = sqlite3_exec(
			handle.get(), /* the db */
			text,
			the_callback.value_or(nullptr),
			nullptr, /* a_param passed here */
			nullptr);

		if (SQLITE_OK != result)
		{
			throw sql_exception{ result, sqlite3_errmsg(handle.get()) };
		}
	}

	}; // connection

	/*
	*/
	inline  auto test_insert ()
	{
	     static const char * db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db";
		try
		{
			connection c;
			c.open(db_file);
			c.execute("create table Hens ( Id int primary key, Name nvarchar(100) not null )");
			c.execute("insert into Hens (Id, Name) values (1, 'Rowena'), (2, 'Henrietta'), (3, 'Constance')");
		}
		catch (sql_exception const & e)
		{
			wprintf(L"dbj::sqlite exception");
			wprintf(L"%d %S\n", e.code, e.message.c_str());
		}
	}

	inline  auto test_select ( callback_type cb_ )
	{
	     static const char * db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db";

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

} // namespace dbj::sqlite
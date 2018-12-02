#pragma once
#include "dbj++sql.h"

namespace dbj_db_test_
{
	using namespace dbj::db;

	struct demo_db final {

		static const database & instance()
		{
			static const database & instance_ = [&]()
				-> const database &
			{
				static database db(":memory:");
				db.query("DROP TABLE IF EXISTS demo");
				db.query("CREATE TABLE demo_table ( Id int primary key, Name nvarchar(100) not null )");
				db.query("INSERT INTO demo_table (Id, Name) values (1, 'London'), (2, 'Glasgow'), (3, 'Cardif')");
				return db;
			}();
			return instance_;
		}
	};

	inline  auto test_insert(const char * = 0)
	{
		try
		{
			const database & db = demo_db::instance();
			db.query(
				u8"INSERT INTO demo_table (Id, Name) "
				u8"values (4, 'Krčedin'), (5, 'Čačak'), (6, 'Kruševac')"
			);
			// please read here about u8 and execution_character_set
			// https://docs.microsoft.com/en-gb/cpp/preprocessor/execution-character-set?view=vs-2017
		}
		catch (sql_exception const & e)
		{
			wprintf(L"dbj::db exception");
			wprintf(L"%d %S\n", e.code, e.message.c_str());
		}
	}

	int sample_callback(
		const size_t row_id,
		[[maybe_unused]] const std::vector<std::string> & col_names,
		const dbj::db::value_decoder & val_user
	)
	{
		int   id_ = val_user(0);
		std::string   name_ = val_user(1);
		::wprintf(L"\n%d      |%d  |%S   ",
			static_cast<int>(row_id), id_, name_.c_str());

		return SQLITE_OK;
	}

	inline  auto test_select()
	{
		try
		{
			const database & db = demo_db::instance();
			::wprintf(L"\n\n"
				L"Row Id |Id |Name");
			::wprintf(L"\n-------+---+--------");
			db.query("SELECT Id,Name FROM demo_table", sample_callback);
			::wprintf(L"\n-------+---+--------\n");
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
			// provoke error
			db.query(
				"select word from words where word like 'bb%'",
				row_user_);
		}
		catch (sql_exception const & e)
		{
			wprintf(L"\ndbj::db exception\n\t[%d] %S\n", e.code, e.message.c_str());
		}
	}
} // nspace

	
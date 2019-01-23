#pragma once

#include "../dbj++sql.h"

namespace dbj_db_test_
{
	using namespace dbj::db;
	using namespace dbj::db::err;

	struct demo_db final {

	// error_code is the ref argument
	static const database & instance( error_code & ec )
	{
		auto initor = [&]()
			-> const database &
		{
			ec.clear();
			static database db(":memory:", ec); if (ec) return db;
			if (ec = db.query("DROP TABLE IF EXISTS demo"); ec) return db;
			if (ec = db.query("CREATE TABLE demo_table ( Id int primary key, Name nvarchar(100) not null )"); ec) return db;
			if (ec = db.query("INSERT INTO demo_table (Id, Name) values (1, 'London'), (2, 'Glasgow'), (3, 'Cardif')"); ec) return db;
			return db;
		};
		static const database & instance_ = initor();
		// note: ec might be not OK here! caller must check!
		return instance_;
	}
};
	/*
	usage is now with no exception being thrown
	NOTE: we pass the error_code out, which is not 
	introcuing unknown abstraction to the caller
	since error_code is part of the std lib
	*/
	[[nodiscard]] inline error_code 
		test_insert(const char * = 0) noexcept
	{
		error_code err_;
		const database & db = demo_db::instance(err_);
		if (err_) {
			// it is already logged
			return err_;
		}
		err_.clear(); // always advisable
		// please read here about u8 and execution_character_set
		// https://docs.microsoft.com/en-gb/cpp/preprocessor/execution-character-set?view=vs-2017
		return db.query(
			u8"INSERT INTO demo_table (Id, Name) "
			u8"values (4, 'Krčedin'), (5, 'Čačak'), (6, 'Kruševac')"
		);
	}
	/*
	remember: this is called once per  each row in the result set
	*/
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
		// otherwise the system will atop
		return SQLITE_OK;
	}

	/*
	no exceptions, but inspect the returned
	*/
	[[nodiscard]] inline error_code test_select() noexcept
	{
			error_code err_;
			const database & db = demo_db::instance(err_);
			if (err_) {
				// it is already logged
				return err_;
			}
			::wprintf(L"\n\n"
				L"Row Id |Id |Name");
			::wprintf(L"\n-------+---+--------");
			err_ = db.query("SELECT Id,Name FROM demo_table", sample_callback);
			::wprintf(L"\n-------+---+--------\n");
			return err_;
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
	[[nodiscard]] inline error_code test_statement_using(
		result_row_user_type row_user_,
		const char * db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"
	) noexcept
	{
		error_code err_;
		const database db(db_file, err_ ); // using the real db
		if (err_) {
			// it is already logged
			return err_;
		}
		// provoke error
		return db.query(
			"select word from words where word like 'bb%'",
			row_user_);
	}
} // nspace


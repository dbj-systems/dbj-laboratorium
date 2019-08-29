#pragma once
#include "../pch.h"

namespace dbj_db_test_
{
	// using namespace  ::std;
	// namespace sql = ::dbj::sql;

	//using status_type = typename dbj::sql::dbj_db_status_type;
	//using buffer = typename dbj::sql::v_buffer;
	//using buffer_type = typename dbj::sql::v_buffer::buffer_type;

	// in memory db for testing
	inline auto const & demo_db( status_type & ec)
		// no throwing from here
		noexcept
	{
		auto initor = [&]()
			-> const sql::database &
		{
			ec.clear();
			// notice the status_type argument 
			// to the cosntructor
			// it does not throw on error
			static sql::database db(":memory:", ec);
			// if error return
			if (ec) return db;
			// create the database and 
			// update the error code
			ec = db.exec(
				"DROP TABLE IF EXISTS demo; "
				"CREATE TABLE demo_table ( Id int primary key, Name nvarchar(100) not null ); "
				"INSERT INTO demo_table (Id, Name) values (1, 'London'), (2, 'Glasgow'), (3, 'Cardif');"
				"INSERT INTO demo_table (Id, Name) values (4, 'Belgrade'), (5, 'Roma'), (6, 'Sarajevo');"
				"INSERT INTO demo_table (Id, Name) values (7, 'Pregrevica'), (8, 'Zemun'), (9, 'Batajnica')"
			);
			return db;
		};
		static  sql::database const& instance_ = initor();
		// note: caller must check the status_type !
		return instance_;
	} // demo_db

		/*
		usage is with no exception being thrown
		NOTE: we pass the status_type out, which is not
		introducing unknown abstraction to the calling site
		std::status_type is part of the std lib
		*/
	[[nodiscard]] inline status_type
		test_insert(const char* = 0) noexcept
	{
		status_type err_;
		const sql::database& db = demo_db(err_);
		if (err_) {
			return err_;
		}
		// please read here about u8 and execution_character_set
		// https://docs.microsoft.com/en-gb/cpp/preprocessor/execution-character-set?view=vs-2017
		return db.exec(
			u8"INSERT INTO demo_table (Id, Name) "
			u8"values (4, 'Krčedin'), (5, 'Čačak'), (6, 'Kruševac')"
		);
	}
	/*
	callback to be called per each row of
	the result set made by:
		SELECT Id,Name FROM demo_table
	Singature of the callback function is always the same
	*/
	int sample_callback(
		const size_t row_id,
		const sql::row_descriptor& cell
	)
	{
		// get the int value of the first column
		int   id_ = cell(0);
		// get the string value of the second column
		buffer_type   name_ = cell(1);
		// print what we got
		
		DBJ_FPRINTF( stdout, "\n\t %zu \t %s = %d \t %s = %s",
			row_id, cell.name(0), id_, cell.name(1), name_.data());

		return SQLITE_OK;
		// we have to use SQLITE_OK macro
		// otherwise sqlite3 will stop the 
		// result set traversal
	}

	int universal_callback(
		const size_t row_id,
		const sql::row_descriptor& cell
	)
	{
		auto print_cell = [&](int j_) {
			DBJ_FPRINTF(stdout, "%10s: %s,", cell.name(j_), ((buffer_type)cell(j_)).data());
		};

		DBJ_FPRINTF(stdout, "\n\t%zu", row_id);
		for (int k = 0; k < cell.column_count(); k++)
			print_cell(k);
		return SQLITE_OK;
		// otherwise sqlite3 will stop the 
		// result set traversal
	}
	/*
	use the above callback's
	*/
	[[nodiscard]] inline status_type
		test_table_info()
		noexcept
	{
		status_type err_;
		const sql::database& db = demo_db(err_);
		if (err_) return err_;

		DBJ_FPRINTF(stdout, "\nmeta data for columns of the table 'demo_table'\n");
		err_.clear();
		err_ = sql::table_info(db, "demo_table", universal_callback);
		if (err_) {
			DBJ_FPRINTF(stdout, "\n%s",	err_.to_buffer(err_).data()	);
		}

		return err_;
	}

	[[nodiscard]] inline status_type
		test_select()
		noexcept
	{
		status_type err_;
		const sql::database& db = demo_db(err_);
		if (err_) {
			// it is already logged
			return err_;
		}
		DBJ_FPRINTF(stdout, "\nexecute: 'SELECT Id, Name FROM demo_table'\n");
		err_ = db.query("SELECT Id,Name FROM demo_table", sample_callback);
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
	[[nodiscard]] inline status_type test_statement_using(
		sql::result_row_callback row_user_,
		const char* db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"
	) noexcept
	{
		status_type err_;
		const sql::database db(db_file, err_); // using the real db
		if (err_) {
			// it is already logged
			// just return it
			return err_;
		}
		err_.clear();
		// provoke error
		return db.query(
			"select word from words where word like 'bb%'",
			row_user_);
	}
} // nspace

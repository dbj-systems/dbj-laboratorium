#pragma once

#include "../dbj++sql.h"

namespace dbj_db_test_
{
	using namespace  ::std;
	namespace sql  = ::dbj::db;
	namespace sqr  = ::dbj::db::err;

// in memory db for testing
inline sql::database const & 
   demo_db( error_code & ec ) 
// no throwing from here
  noexcept
{
auto initor = [&]()
	-> const sql::database &
{
	ec.clear();
	// notice the error_code argument 
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
"INSERT INTO demo_table (Id, Name) values (1, 'London'), (2, 'Glasgow'), (3, 'Cardif')"); 
	return db; 
};
static  sql::database const & instance_ = initor();
// note: caller must check the error_code !
return instance_;
} // demo_db

	/*
	usage is with no exception being thrown
	NOTE: we pass the error_code out, which is not 
	introcuing unknown abstraction to the caller
	since error_code is part of the std lib
	*/
	[[nodiscard]] inline error_code 
		test_insert(const char * = 0) noexcept
	{
		error_code err_;
		const sql::database & db = demo_db(err_);
		if (err_) {
			// it is already logged
			return err_;
		}
		err_.clear(); // always advisable
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
	const sql::row_descriptor & cell
)
{
	// get the int value of the first column
	int   id_ = cell(0);
	// get the string value of the second column
	string   name_ = cell(1);
	// print what we got
	::wprintf(L"\n\t %zu \t %S = %d \t %S = %S",
		row_id, cell.name(0), id_, cell.name(1), name_.c_str());
	return SQLITE_OK;
	// otherwise sqlite3 will stop the 
	// result set traversal
}

int universal_callback(
	const size_t row_id,
	const sql::row_descriptor & cell
)
{
	auto print_cell = [&](int j_) {
		::wprintf(L"%10S: %S,", cell.name(j_), ((string)cell(j_)).c_str() );
	};

	::wprintf(L"\n\t%zu", row_id);
	for (int k = 0; k < cell.column_count(); k++)
		print_cell(k);
	return SQLITE_OK;
	// otherwise sqlite3 will stop the 
	// result set traversal
}
/*
use the above callback's
*/
[[nodiscard]] inline error_code 
test_table_info () 
noexcept
{
	error_code err_;
	const sql::database & db = demo_db(err_);
	if (err_) return err_;

	::wprintf(L"\nmeta data for columns of the table 'demo_table'\n");
	err_.clear();
	err_ = sql::table_info(db, "demo_table", universal_callback);
	if (err_)
	::wprintf(L"\n\nstd::error_code domain:%S, id:%d, message:%S ", err_.category().name(), err_.value(), err_.message().c_str());

	return err_;
}
[[nodiscard]] inline error_code 
test_select() 
noexcept
{
	error_code err_;
	const sql::database & db = demo_db(err_);
	if (err_) {
		// it is already logged
		return err_;
	}
	::wprintf(L"\nexecute: 'SELECT Id, Name FROM demo_table'\n");
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
	[[nodiscard]] inline error_code test_statement_using(
		sql::result_row_callback row_user_,
		const char * db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"
	) noexcept
	{
		error_code err_;
		const sql::database db(db_file, err_ ); // using the real db
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


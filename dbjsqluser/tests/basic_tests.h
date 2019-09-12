#pragma once
/*
(c) 2019 by dbj@dbj.org -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/
*/  
#include "test_db.h"


namespace dbj_sql_user
{
	/*
	no exceptions are thrown
	we return the sql::status_and_location, and
	we make sure it is not discarded
	*/
	[[nodiscard]] inline sql::status_and_location test_wrong_insert(sql::database const& db) noexcept
	{

		/*
		 here we make sure we insert utf8 encoded ANSI string literals
		 please read here about u8 and execution_character_set
		 https://docs.microsoft.com/en-gb/cpp/preprocessor/execution-character-set?view=vs-2017

		 note: db.exec return the status too

		 this SQL is wrong because we already have 4,5,6 as primary keys in that table in the demo db
		 thus the caller will be notified and will decide what to do
		 */
		constexpr auto BAD_SQL = u8"INSERT INTO entries (Id, Name) "
			u8"values (4, 'Krčedin'), (5, 'Čačak'), (6, 'Kruševac')";

		DBJ_PRINT("\nAttempting BAD SQL: %s\n", BAD_SQL);
				return db.exec(BAD_SQL);
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

		DBJ_PRINT( "\n\t %zu \t %s = %d \t %s = %s",
			row_id, cell.name(0), id_, cell.name(1), name_.data());

		return SQLITE_OK;
		// we have to use SQLITE_OK macro
		// otherwise sqlite3 will stop the 
		// result set traversal
	}


	/*
	use the universal callback provided by dbj++sqlite
	*/
	[[nodiscard]] inline sql::status_and_location test_table_info( sql::database const& db ) noexcept
	{
		DBJ_PRINT( "\nmeta data for columns of the table 'entries'\n");
		/*
		execute the table_info and pass the status out
		*/
		return sql::table_info(db, "entries", sql::universal_callback);
	}

	[[nodiscard]] inline sql::status_and_location test_select(sql::database const& db) noexcept
	{
		DBJ_PRINT( "\nexecute: 'SELECT Id, Name FROM entries'\n");
		return db.query("SELECT Id,Name FROM entries", sample_callback);
	}

	/*
	This is how dbj++sqlite helps you consume the sqlite3 query results
	Remember: this is called once per each row
	SQL statement result set this callback is processing is:
	"select word, definition from entries where word like 'zyga%'"
	*/
	int example_callback(
		const size_t row_id,
		const dbj::sql::row_descriptor& row_
	)
	{
		// 'automagic' transform to the buffer type
		// of the column 0 value for this row
		// auto can not be used here
		// compiler would not know what type you want
		buffer_type  word_ = row_(0);
		buffer_type  definition_ = row_(1);
		DBJ_PRINT( "\n\n%3zd: %12s | %s", row_id, word_.data(), definition_.data());

		//// all these should provoke exception
		//// TODO: but they don't -- currently
		//long   DBJ_MAYBE(number_) = row_(0);
		//double DBJ_MAYBE(real_) = row_(0);

		return SQLITE_OK;
	}

	/* here we use the external database
	the DB_FILE_PATH is set to its full path
	*/
	TU_REGISTER(
		[] {
			sql::status_and_location status_{};
			// 
			sql::database db(DICTIONARY_DB_FILE_PATH, status_);
			// some kind of error has happened
			if ( is_error( status_ )) {
				DBJ_PRINT("\n\n ERROR Status : \n %s\nWhile opening the database: %s\n", to_json(status_).data(), DICTIONARY_DB_FILE_PATH);
				return ;
			}

			constexpr auto SQL = "SELECT word, definition FROM entries WHERE word LIKE 'zyga%'";
			DBJ_PRINT("\n\nExternal database: %s, testing the query: %s", db.db_name(), SQL);
			// returns the status
			status_ = db.query(SQL, example_callback);

			if (is_error(status_)) {
				DBJ_PRINT("\n\n ERROR Status : \n\n %s\n\nWhile querying the database: %s\n", to_json(status_).data(), db.db_name());
			}
		});

	/*
	Test Unit registration
	*/
	TU_REGISTER(
		[] {
			sql::status_and_location status_;
				sql::database const & db = demo_db(status_);
				// some kind of error has happened
				if (is_error(status_)) {
					DBJ_PRINT( "\n\n ERROR Status : \n %s\n\n", to_json(status_).data());
					return;
				}

			status_ = test_wrong_insert( db );
			if (is_error(status_)) DBJ_PRINT( "\n\n test_wrong_insert()\tStatus : \n %s\n\n", to_json(status_).data());

			status_ = test_table_info( db );
			if (is_error(status_)) DBJ_PRINT( "\n\n test_table_info()\tStatus : \n %s\n\n", to_json(status_).data());

			status_ = test_select( db );
			if (is_error(status_)) DBJ_PRINT( "\n\n test_select()\tStatus : \n %s\n\n", to_json(status_).data());

			/*
			NOTE: above we just perform "print-and-proceed"
			usually callers will use status_ to develop another logic
			*/
		});

} // eof two_tests namespace
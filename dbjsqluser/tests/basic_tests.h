#include "test_db.h"

namespace dbj_sql_user
{
	/*
	no exceptions are thrown
	we return the status_type, and
	we make sure it is not discarded
	*/
	[[nodiscard]] inline status_type test_wrong_insert() noexcept
	{
		/*
		this is different use pattern
		we create the status type in here vs receiving it
		as ref. argument
		*/
		status_type err_;
		/*
		ref. to the single instance
		*/
		sql::database const& db = demo_db(err_);
		/*
		there is no point proceeding if in the error state
		*/
		if (err_) return err_;

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

		DBJ_PRINT("\nAttempting: %s", BAD_SQL);
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

		DBJ_FPRINTF(stdout, "\n\t %zu \t %s = %d \t %s = %s",
			row_id, cell.name(0), id_, cell.name(1), name_.data());

		return SQLITE_OK;
		// we have to use SQLITE_OK macro
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

		DBJ_FPRINTF(stdout, "\nmeta data for columns of the table 'entries'\n");
		err_.clear();
		/*
		execute the table_info and pass the status out
		*/
		return sql::table_info(db, "entries", sql::universal_callback);
	}

	[[nodiscard]] inline status_type test_select() noexcept
	{
		status_type err_;
		const sql::database& db = demo_db(err_);
		if (err_) 	return err_; // return on error

		DBJ_FPRINTF(stdout, "\nexecute: 'SELECT Id, Name FROM entries'\n");
		return db.query("SELECT Id,Name FROM entries", sample_callback);
	}

	/*
	Remember: this is called once per each row
	SQL statement result set this is processing is:

	"select word, definition from entries where word like 'zy%'"
	*/
	int example_callback(
		const size_t row_id,
		const dbj::sql::row_descriptor& row_
	)
	{
		// 'automagic' transform to the buffer type
		// of the column 0 value for this row
		buffer_type  word_ = row_(0);
		buffer_type  definition_ = row_(1);
		DBJ_FPRINTF(stdout, "\n\n%3zd: %12s | %s", row_id, word_.data(), definition_.data());

		//// all these should provoke exception
		//// TODO: but they don't -- currently
		//long   DBJ_MAYBE(number_) = row_(0);
		//double DBJ_MAYBE(real_) = row_(0);

		return SQLITE_OK;
	}

	/* here we use the external database
	the DB_FILE_PATH is set to its full path
	*/
	[[nodiscard]] inline status_type test_statement_using(
		sql::result_row_callback row_user_,
		const char* db_file = DB_FILE_PATH.data()
	) noexcept
	{
		status_type status;
		const sql::database db(db_file, status); // using the real db from the db file
		if (status) return status;

		// returns the status
		return db.query("select word, definition from entries where word like 'zy%'", row_user_);
	}



	/*
	Test Unit registration
	*/
	TU_REGISTER(
		[] {
			::dbj::sql::dbj_db_status_type  status_;

			status_ = test_wrong_insert();
			DBJ_FPRINTF(stdout, "\n\n Status : \n %s\n\n", status_.c_str());

			status_ = test_table_info();
			DBJ_FPRINTF(stdout, "\n\n Status : \n %s\n\n", status_.c_str());

			status_ = test_select();
			DBJ_FPRINTF(stdout, "\n\n Status : \n %s\n\n", status_.c_str());

			status_ = test_statement_using(example_callback);
			DBJ_FPRINTF(stdout, "\n\n Status : \n %s\n\n", status_.c_str());

			/*
			NOTE: above we just perform "print-and-proceed"
			usually callers will use it to develop another logic
			if there is an error they wish to process and act
			on the status_
			*/
		});

} // eof two_tests namespace
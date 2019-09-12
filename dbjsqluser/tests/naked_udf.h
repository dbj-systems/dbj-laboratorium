/*
(c) 2019 by dbj@dbj.org -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/
*/  
#pragma once

#include "test_db.h"

namespace dbj_sql_user {

	/*
	------------------------------------------------------------
	An example on how to do native SQLITE UDF on top of dbj++sql 
	*/

	/* using namespace ::sqlite; <-- this is where sqlite3 API lives in dbj++ world */
	using namespace sqlite;

	/* 
	SQLITE UDF is just an callback function, with required siganture 
	this is an example of coding it with native sqlite3 api
	notice hwo this is C++ with mangled function name too
	*/
	inline void firstchar_udf (sqlite3_context* context, int argc, sqlite3_value** argv)
	{
		if (argc == 1) {

			const unsigned char* text = sqlite3_value_text(argv[0]);

			if (text && text[0]) {
				char result[2]{ '\0' };
				result[0] = text[0]; 
				sqlite3_result_text(context, result, -1, SQLITE_TRANSIENT);
				return;
			}
		}
		sqlite3_result_null(context);
	}

	TU_REGISTER([] {

		/*
		Getting the dbj++sql database
		*/
		sql::status_and_location status;
			const sql::database& database = demo_db(status);
				CHECK_RETURN;

		DBJ_PRINT("\n\nTesting SQLITE3 native UDF, with the help of dbj++sqlite");

		// the native sqlite3 database handle
		sqlite3 * db = database.the_db();

		// C++17 if syntax
		if (
			int sqlite3_retval = sqlite3_create_function(db, "firstchar", 1, SQLITE_UTF8, NULL, &firstchar_udf, NULL, NULL);
			sqlite3_retval != SQLITE_OK
			)
		{
			DBJ_PRINT("\n\nSQLITE error while trying to register a calback function ");
			DBJ_PRINT("\nError message: %s", sql::err_message_sql(sqlite3_retval). data() );

		} else {

			status = database.query
			(
				"SELECT firstchar(Name) from entries",
				// you can provide your own sql result handling callback
				// we use ours right now
				sql::universal_callback
			);

			DBJ_PRINT("\n\nDone with 'SELECT firstchar(Name) from entries'");
			DBJ_PRINT("\n\nStatus returned is: %s\n", to_json( status ).data() );
		}

		});

} // pure_udf
#undef CHECK_RETURN

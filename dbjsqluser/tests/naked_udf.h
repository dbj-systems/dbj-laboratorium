/*
(c) 2019 by dbj@dbj.org -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/
*/
#pragma once

#include "test_db.h"

namespace dbj_sql_user {

	/*
	------------------------------------------------------------
	An example on how to do native SQLITE UDF on top of dbj++sql
	while coding in C++17

	using namespace ::sqlite; <-- this is where sqlite3 API lives in dbj++ world 
	*/
	using namespace sqlite;

	/*
	SQLITE UDF is just an callback function, with required siganture
	this is an example of coding it with native sqlite3 api
	notice how this is C++ with mangled function name too
	*/
	inline void firstchar_udf(sqlite3_context* context, int argc, sqlite3_value** argv)
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

	/*
	------------------------------------------------------------
	Test Unit
	*/
	TU_REGISTER([] {

		DBJ_PRINT("\n\nUsing SQLITE3 native UDF, with a little help of dbj++sqlite\n\n");

		/* can be function or lambda or functor */
		auto native_callback = [](void* row_, int argc, char** argv, char** col_names)
			->int
		{
			if (row_ == nullptr) return SQLITE_ABORT;

			sql::cursor_iterator* cursor_ = (sql::cursor_iterator*)row_;
			auto value = cursor_->to_text(0);

			for (int i = 0; i < argc; ++i)
				DBJ_PRINT("\n%s = %s", col_names[i], (*value).data());
			return SQLITE_OK;
		};
		/*
		Getting the demo database
		*/
		auto [db, status] = demo_db();
		// error
		if (!db) {
			DBJ_PRINT_STATUS(status);
			return; 
		}

		// db type is optional<reference_wrapper< sql::database >>
		sql::database const& database = *db;

		int retval =
			sqlite3_create_function(database.the_db(), "firstchar", 1, SQLITE_UTF8, NULL, &firstchar_udf, NULL, NULL);

		if (SQLITE_OK != retval) return;

		auto SQL = "SELECT firstchar(Name) FROM entries";
		/*
		Instead of query() we shall use the exec() metohod
		*/
		sql::print_on_sql_error(database.exec(SQL, native_callback));

		DBJ_PRINT("\n\nDone\n\n");

		});

} // pure_udf
#undef DBJ_CHECK_RETURN

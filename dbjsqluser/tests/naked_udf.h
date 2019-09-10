#pragma once
#include "test_db.h"

#if 0
namespace dbj_sql_user {

	/* 
	this is SQLITE3 UDF in its canonical form 

	sqlite3_context -- https://sqlite.org/c3ref/context.html
	sqlite3_value -- https://sqlite.org/c3ref/value.html
	*/
	extern "C" static void palindrome(
		sqlite::sqlite3_context* context,
		int argc,
		sqlite::sqlite3_value** argv
	)
	{
		sqlite::sqlite3_result_null(context);
		// if no arguments just return
		if (argc < 1) return;

		// we are expecting the first qrgument to be TEXT 
		if (sqlite::sqlite3_value_type(argv[0]) != SQLITE_TEXT) return;

		// take the value as a text 
		char* text = (char*)sqlite::sqlite3_value_text(argv[0]); DBJ_VERIFY(text);
		// take the length of a text 
		const size_t text_length = sqlite::sqlite3_value_bytes(argv[0]); DBJ_VERIFY(text_length > 0);
		// store it to the std string
		std::string word_{ text, text_length };
		static int result = 0; // aka 'false'
		// check if word is a palindrome
		result = is_pal(word_.c_str());
		// pass the result to the sqlite
		sqlite::sqlite3_result_int(context, result);
		return;
	}

	inline status_type test_udf
	(
		sql::database const& db,
		std::string_view query_
		// = "SELECT palindrome(Name) FROM entries"
		= "SELECT * FROM entries WHERE (1 == palindrome(Name))"
	)
	{
		status_type  status =
			// here we register our C++ UDF, associate with a name
			// to be used from SQL
			db.register_user_defined_function("palindrome", palindrome);

		if (status) return status; // error

		DBJ_PRINT("\nQuery: '%s'\n", query_.data());

		// execute the statement
		// with the appropriate callback used
		status = db.query
		(
			query_.data(),
			sql::universal_callback
		);

		DBJ_PRINT("\nDone...\n");

		return status;
	}

	/*
	Test Unit registration
	*/
	TU_REGISTER([] {

		status_type status{};
		
		sql::database const& database = demo_db(status)
		CHECK_RETURN;

		// just check the :memory: demo db is here and in shape we need it to be
		status = test_udf(database, "SELECT * FROM entries;");
		CHECK_RETURN;

		status = test_udf(database);
		CHECK_RETURN;
		});
} // naked_udf
#endif

namespace dbj_sql_user {

	/* using namespace ::sqlite; <-- this is where sqlite3 API is */
	using namespace sqlite;

	inline void firstchar_udf (sqlite3_context* context, int argc, sqlite3_value** argv)
	{
		if (argc == 1) {

			const unsigned char* text = sqlite3_value_text(argv[0]);

			if (text && text[0]) {
				char result[2];
				result[0] = text[0]; result[1] = '\0';
				sqlite3_result_text(context, result, -1, SQLITE_TRANSIENT);
				return;
			}
		}
		sqlite3_result_null(context);
	}

	TU_REGISTER([] {
		status_type status;
			const sql::database& database = demo_db(status);
				CHECK_RETURN;

		// the pure sqlite3 API usage
		auto db = database.the_db();
		int sqlite3_retval = sqlite3_create_function(db, "firstchar", 1, SQLITE_UTF8, NULL, &firstchar_udf, NULL, NULL);

		auto dbj_sql_status = database.query
		(
			"SELECT firstchar(Name) from entries",
			sql::universal_callback
		);

		DBJ_PRINT("\n\nDone with 'SELECT firstchar(Name) from entries'");

		});

} // pure_udf
#undef CHECK_RETURN

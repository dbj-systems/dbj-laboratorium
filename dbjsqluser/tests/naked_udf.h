#pragma once
#include "../pch.h"

namespace naked_udf {

	/*
	using namespace ::sqlite; <-- this is where sqlite3 API is	sans macros of course
	*/
	using namespace ::std;
	using namespace ::std::string_view_literals;

	/*	the dbj++sql namespace	*/
	namespace sql = ::dbj::sql;

	using status_type = typename dbj::sql::dbj_db_status_type;

	using buffer = typename dbj::nanolib::v_buffer;
	using buffer_type = typename buffer::buffer_type;

	constexpr inline auto DEMO_DB_CREATE_SQL = "DROP TABLE IF EXISTS entries; "
		"CREATE TABLE entries ( Id int primary key, Name nvarchar(100) not null ); "
		"INSERT INTO entries values (1, 'LondonodnoL');"
		"INSERT INTO entries values (2, 'Glasgow');"
		"INSERT INTO entries values (3, 'CardifidraC');"
		"INSERT INTO entries values (4, 'Belgrade');"
		"INSERT INTO entries values (5, 'RomamoR');"
		"INSERT INTO entries values (6, 'Sarajevo');"
		"INSERT INTO entries values (7, 'Pregrevica');"
		"INSERT INTO entries values (8, 'ZemunumeZ');"
		"INSERT INTO entries values (9, 'Batajnica');";

	// return true if argument str
		// is a pointer to palindrome
		// false otherwise
	inline bool is_pal(const char* str) {
		char* s = (char*)str;
		char* e = (char*)str;
		//
		while (*e) e++;
		--e;
		while (s < e) {
			if (*s != *e) return false;
			++s;
			--e;
			if (e < s) break;
		}
		return true;
	}

	/* 
	this is SQLITE3 UDF in its canonical form 

	sqlite3_context -- https://sqlite.org/c3ref/context.html
	sqlite3_value -- https://sqlite.org/c3ref/value.html
	*/
	inline void palindrome(
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
		= "SELECT word FROM entries WHERE (1 == palindrome(word))"
	)
	{
		status_type  status =
			// here we register our C++ UDF, associate with a name
			// to be used from SQL
			db.register_user_defined_function("palindrome", palindrome);

		if (status) return status; // error

		// execute the statement
		// with the appropriate callback used
		return db.query
		(
			query_.data(),
			sql::universal_callback
		);
	}

#define CHECK_RETURN if (status) { DBJ_FPRINTF(stdout, "\n\n%s\n\n", (char const*)status); return; }
	/*
	Test Unit registration
	*/
	TU_REGISTER([] {

		status_type status;

		// this is called only on the firs call
		auto initor = [](status_type& stat_)
			->  sql::database &
		{
			static sql::database db(":memory:", stat_);
			if (stat_) return db; // return on error state
			stat_ = db.exec(DEMO_DB_CREATE_SQL);
			return db;
		};
		// here we keep the single sql::database type instance
		static  sql::database& database = initor(status);
		CHECK_RETURN;

		// just check the :memory: demo db is here and in shape we need it to be
		status = test_udf(database, "SELECT * FROM entries;");
		CHECK_RETURN;

		status = test_udf(database);
		CHECK_RETURN;
		});
#undef CHECK_RETURN
} // naked_udf

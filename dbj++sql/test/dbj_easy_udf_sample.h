#pragma once

#include "../dbj++sql.h"

/*
 SQL user defined functions aka UDF
 are parts of SQL statement defined by users
 in RDBMS's they are to be written in SQL language too
 and stored with special SQL , etc ...
 in SQLITE they are just C or C++ code, since the whole
 SQLITE is just a C library
 user just has to use the appropriate SQLITE3 functions
 to assign them after writing them in her code
 which is for a large portion of sqlite3 users intimidating
 unless the use the DBJ EASY UDF's API

	// dbj easy udf c++ signature
	void my_udf(
		const udf_argument  & value_,
		const udf_retval    & result_
	) ;

	see the two examples bellow
*/

namespace dbj_easy_udfs_sample {

	using namespace ::std;
	using namespace ::std::string_view_literals;
	namespace sql = ::dbj::db;
	namespace err = ::dbj::db::err;

	/* one table "words", with one text column "word" */
	constexpr inline auto DB_FILE = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"sv;

	/* use case: solve the following query using
	   dbj++sql,  the dbj sqlite 3 C++ api
	   notice the two udf's required
	   int palindrome( const char *) and int strlen( const char *)
	   they are not available in SQLITE3 SQL as inbuilt functions
	*/
	constexpr inline auto QRY_WITH_UDF
		= "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"sv;
	/*
	the result set: 0:'word'(type:const char *), 1:'strlen(word)'(type:int)

	Please observe and understand the shape of the sql select, result set
	as this is from where we source the argument for the palindrome()
	the palindrome udf has to be:  int  palindrome ( const char * )
	we write it using the dbj easy pdf function signature
	alsways the same for any udf
	*/
	inline void palindrome(
		const sql::udf_argument  & value_,
		const sql::udf_retval    & result_
	)
		// do not throw from the UDF
		noexcept
	{
		auto is_pal = [](const char* str) constexpr -> bool
		{
			char* s = (char*)str;
			char* e = (char*)str;
			while (*e) e++;
			--e;
			while (s < e) {
				if (*s != *e) return false;
				++s;
				--e;
				if (e < s) break;
			}
			return true;
		};
		/*	udf_argument and udf_retval types have all we need to pass
			the values in and out of udf as required by the SQL statement
		*/
		string word_ = value_(0);
		int result = is_pal(word_.c_str());
		result_(result);
	}

	/* the strlen udf :  int strlen( const char *)
	   to be used in this statement
	   "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
	*/
	inline void strlen_udf (
		const sql::udf_argument  & argument_,
		const sql::udf_retval    & result_
	)
		// do not throw from the UDF
		noexcept
	{
		string word_ = argument_(0);
		int result = (int)(word_.size());
		result_(result);
	}

	/* the standard dbj++sql result processor aka callback
	   to handle the final sql select, result set
	*/
	static int dbj_udfs_result_handler(
		const size_t row_id,
		const sql::row_descriptor & col
	)
	{
		/* this is deliberately verbose code */
		string word = col(0);
		int len_ = col(1);
		::wprintf(L"\n\t[%3zu]\tword: %32S,\t%S: %12d",
			row_id, word.c_str(), col.name(1), len_);
		// make sure to return always this
		return SQLITE_OK;
	}

	/* to be called from your test unit
	   here we register the two easy udf's we made above
	   we do not throw, we use std::error_code
	*/
	[[nodiscard]] error_code test_udf(
		string_view query_ = QRY_WITH_UDF
	) noexcept
	{
		::wprintf(L"\n\nthe query:\t%S\nthe result:\n\n", query_.data());
		// assure the database presence
		// return error_code on error
		error_code ec;
		sql::database db(DB_FILE, ec); if (ec) return ec;
		// register the two udf's required
		// always returning the error_code on error
		// obviuosly macro afficionados are welcome here
		ec.clear();
		ec = sql::register_dbj_udf<palindrome>(db, "palindrome"); if (ec) return ec;
		ec.clear();
		ec = sql::register_dbj_udf<strlen_udf>(db, "strlen"); if (ec) return ec;
		// above we could use any names we want, 
		// as long as they match the sql select used

		// execute the query using the 
		// standrd result processing calback 
		return db.query(query_.data(), dbj_udfs_result_handler);
		// return the error_code
	} // test_udf

} // namespace dbj_easy_udfs_sample 

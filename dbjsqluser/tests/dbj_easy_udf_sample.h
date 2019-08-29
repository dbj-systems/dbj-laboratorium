#pragma once
#include "../pch.h"

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
		const udf_argument  & args_,
		const udf_retval    & result_
	) ;

	dbj++sql makes sqlite udf's realy easy, see the two examples bellow
*/

namespace dbj_easy_udfs_sample {

	using namespace ::std;
	using namespace ::std::string_view_literals;
	
	// DB_FILE_PATH setup
	// current default:
	// setup of constexpr inline auto DB_FILE_PATH = "d:\\dictionary.db"sv;

#include "../db_file_path.inc"


	/* use case: solve the following query using
	   notice the two udf's required
	   int palindrome( const char *) and int strlen( const char *)
	   they are not available in SQLITE3 SQL as inbuilt functions
	*/
	constexpr inline auto QRY_WITH_UDF
		= "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"sv;
	/*
	for the above sql the result set will be:
	0:'word'(type:const char *), 1:'strlen(word)'(type:int)

	Please observe and understand the shape of the sql select, result set
	as this is from where we source the argument for the palindrome()

	the palindrome udf has to be:  int  palindrome ( const char * )
	we write it using the dbj easy udf, function signature
	alsways the same for any udf
	*/
	inline void palindrome(
		const sql::udf_argument& args_,
		const sql::udf_retval& result_
	)
		// do not throw from this UDF
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

			first and only argument is a string
		*/
		auto optional_transformer = args_(0);
		if (!optional_transformer) return;

		// can not use auto here
		buffer_type word_ = *optional_transformer;

		if (word_.size() > 1 )
		{
			/*
			as per sql statement requirements we need to reurn an int
			1 means yes that is a palindrome
			*/
			result_( is_pal(word_.data()) );
		}

		result_( 0 );
	}

	/* the strlen udf :  int strlen( const char *)
	   to be used in this statement
	   "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
	*/
	inline void strlen_udf(
		const sql::udf_argument& args_,
		const sql::udf_retval& result_
	)
		// do not throw from the UDF
		noexcept
	{
		auto optional_transformer = args_(0);
		if (!optional_transformer) return;
		
		buffer_type word_ = *optional_transformer;
		result_((int)(word_.size()));
	}

	/* the standard dbj++sql result processor aka callback
	   to handle the final sql select, result set
	*/
	static int dbj_udfs_result_handler(
		const size_t row_id,
		const sql::row_descriptor& col
	)
	{
		/* this is deliberately verbose code */
		buffer_type word = col(0);
		int len_ = col(1);
		DBJ_FPRINTF( stdout, "\n\t[%3zu]\tword: %32s,\t%s: %12d",
			row_id, word.data(), col.name(1), len_);
		// make sure to return always this
		return SQLITE_OK;
	}

	/* to be called from your test unit
	   here we register the two easy udf's we made above

	   notice how we do not throw, we use std::status_type's
	   emanating from dbj++sql
	*/
	[[nodiscard]] status_type test_udf(
		string_view query_ = QRY_WITH_UDF
	) noexcept
	{
		DBJ_FPRINTF( stdout, "\n\nthe query:\t%s\nthe result:\n\n", query_.data());
		// assure the database presence
		// return status_type on error
		status_type ec;
		sql::database db(DB_FILE_PATH, ec); if (ec) return ec;
		// register the two udf's required
		// string names of udf's must match the SQL they are part of
		// always returning the status_type on error
		// obviuosly macro afficionados are welcome here
		ec.clear();
		ec = sql::register_dbj_udf<palindrome>(db, "palindrome"); if (ec) return ec;
		ec.clear();
		ec = sql::register_dbj_udf<strlen_udf>(db, "strlen"); if (ec) return ec;

		// execute the query using the 
		// standard result processing calback 
		// "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
		return db.query(query_.data(), dbj_udfs_result_handler);
		// return the status_type
	} // test_udf

} // namespace dbj_easy_udfs_sample 
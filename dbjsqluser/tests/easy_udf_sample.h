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

	/*
	the dbj++sql namespace
	*/
	namespace sql = ::dbj::sql;

	/*
	return value type, has 3 std::options

	sqlite states code
	std errc status code
	location inf0
	*/
	using status_type = typename dbj::sql::dbj_db_status_type;

	/*
	std::string is very slow, we use std::vector<char> as a buffer
	std::unique_ptr<char[]> is the only other type we could use
	but it is only slightly faster and not very easy to use
	*/
	using buffer = typename dbj::sql::v_buffer;
	using buffer_type = typename dbj::sql::v_buffer::buffer_type;

	// DB_FILE_PATH setup
	// current default:
	// setup of constexpr inline auto DB_FILE_PATH = "d:\\dictionary.db"sv;
#include "../db_file_path.inc"


	/* use case:
	   solve the following query using dbj++sql easy edf feature

	   notice the two udf's required
	   int palindrome( const char *) and int strlen( const char *)
	   they are not available in SQLITE3 SQL as inbuilt functions
	*/
	constexpr inline auto QRY_WITH_UDF
		= "SELECT word, strlen(word) FROM entries WHERE (1 == palindrome(word))"sv;
	/*
	Please observe and understand the shape of the sql select, result set
	as this is from where we source the argument for the palindrome()

	for the above sql, each row of the result set will be:

	0:'word'(type:const char *), 1:'strlen(word)'(type:int)

	that is: only the plaindromes will be returned and thier strlen
	will be displayed too

	the palindrome udf has to be:  int  palindrome ( const char * )
	bellow we write it using the dbj easy udf, function signature
	has to be always the same for any udf

	inline void user_defined_udf (
	   // the input
		const sql::udf_argument & ,
		// the output
		const sql::udf_retval &
	)

	udf begins here
	*/
	inline void palindrome(
		const sql::udf_argument& args_, // input
		const sql::udf_retval& result_  // output
	)
		// do not throw from this UDF
		noexcept
	{
		// lambda: return true if word is a palindrome
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

			we are writing 'palindrom( word )' udf
			first and only argument is a string passed into here from sqlite
			as the result of procssing this SQL statement:

			"SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"

			thus there is only a single argument
			we will now get to it, bit in a safe fashion
			we must not throw exceptions from here
		*/
		auto optional_transformer = args_(0);
		/*
		if we made a call with the wrong index above the return value
		will be nullopt
		we will simly return on that ocassion
		*/
		if (!optional_transformer) return;

		// can not use auto here
		buffer_type word_ = *optional_transformer;

		if (word_.size() > 1)
		{
			/*
			as per sql statement requirements we need to reurn an int
			1 means yes that is a palindrome
			*/
			result_(is_pal(word_.data()));
		}

		/*
		0 means no that, word given is not a palindrome
		*/
		result_(0);
	}

	/* the strlen udf :  int strlen( const char *)
	   example: to be used in this statement
	   "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
	*/
	inline void strlen_udf(
		const sql::udf_argument& args_, // input
		const sql::udf_retval& result_  // output
	)
		// do not throw from the UDF
		noexcept
	{
		auto optional_transformer = args_(0);
		if (!optional_transformer) return;

		buffer_type word_ = *optional_transformer;
		/*
		note: we need to cast to int, SQL has no size_t
		*/
		result_((int)(word_.size()));
	}

	/* the standard dbj++sql result processor aka callback
	to handle the final sql select, result set

	"SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
	SQL of which the result set will be proecessed here

	for the above sql, each row of the result set will be:
	0:'word'(type:const char *), 1:'strlen(word)'(type:int)

	that is: only the plaindromes will be returned and thier strlen 
	will be displayed too
	*/
	static int dbj_udfs_result_handler(
		const size_t row_id,
		const sql::row_descriptor& col
	)
	{
		/* this is deliberately verbose code */
		buffer_type word = col(0);
		int len_ = col(1);
		
		DBJ_FPRINTF(stdout, "\n\t[%3zu]\tword: %32s,\t%s: %12d",
			row_id, word.data(), col.name(1), len_);
		
		// make sure to return always this  sqlite3 macro
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
		// assure the database presence
		// return status_type on error
		status_type status;
		sql::database db(DB_FILE_PATH, status); 
		// return on error status
		if (status) return status;
		// register the two udf's required
		// string names of udf's must match the SQL they are part of
		// 	"SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
		// always returning the status_type on error
		// obviuosly macro afficionados are welcome here
		status.clear();
		status = sql::register_dbj_udf<palindrome>(db, "palindrome"); 
		if (status) return status; // return on error status
		
		status.clear();
		status = sql::register_dbj_udf<strlen_udf>(db, "strlen"); 
		if (status) return status; // return on error status

		// execute the query using the 
		// standard result processing calback 
		// "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
		return db.query(query_.data(), dbj_udfs_result_handler);
		// return the status_type
	} // test_udf

	TU_REGISTER([] {

		auto status_ = test_udf();

		DBJ_FPRINTF(stdout, "\n%s", (char const*)status_);

		});

} // namespace dbj_easy_udfs_sample 
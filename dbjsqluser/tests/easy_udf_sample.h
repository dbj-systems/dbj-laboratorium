/*
(c) 2019 by dbj@dbj.org -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/
*/
#pragma once

#include "test_db.h"

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

namespace dbj_sql_user {

	/* use case:
	   solve the following query using dbj++sql easy edf feature

	   notice the two udf's required
	   int palindrome( const char *) and int strlen( const char *)
	   they are not available in SQLITE3 SQL as inbuilt functions

		"SELECT Name, strlen(Name) FROM entries WHERE ( 1 == palindrome(Name) );";

		Please observe and understand the shape of the sql select, result set
	as this is from where we source the argument for the palindrome()

	for the above sql, each row of the result set will be:

	0:'word'(type:const char *), 1:'strlen(word)'(type:int)

	that is: only the plaindromes will be returned and thier strlen
	will be displayed too

	the palindrome udf has to be:  int  palindrome ( const char * )
	bellow we write it using the dbj easy udf, function signature
	which always has to be the same for any udf

	inline void user_defined_udf (
	   // the input
		const sql::udf_argument & ,
		// the output
		const sql::udf_retval &
	)

	notice how sqlite3 api can work with C++ inline functions with mangled names

	dbj++sqlite udf begins here
	*/
	inline void palindrome(
		const sql::udf_argument& args_, // input
		const sql::udf_retval& result_  // output
	)
		// do not throw from this UDF
		noexcept
	{
		// return true if argument str, is a pointer to palindrome, false otherwise
		auto is_pal = [](const char* str) constexpr -> bool {
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
		};
		/*
			we are writing 'palindrome( Name )' udf
			first and only argument is a string passed into here from sqlite
			as the result of processing this SQL statement:

			"SELECT * FROM entries WHERE (1 == palindrome(Name))"

		thus there is only a single argument
		we will now get to it, in a safe fashion
		can not use auto here
		dbj::sql::transformer in action
		*/
		buffer_type word_ = args_(0);

		if (word_.size() > 1)
		{
			/*
			as per sql statement requirements we need to reurn an int
			1 means yes that is a palindrome
			*/
			char const * name = word_.data();
			int isit = is_pal( name );
			result_(isit);
			return;
		}
		/*	0 means no , word given is not a palindrome	
		    always, must use result_() before return
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
		buffer_type word_ = args_(0);
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
		const sql::cursor_iterator& col
	)
	{
		/* 
		this is deliberately verbose code 
		you might find it usefull to see
		the types involved
		*/
		buffer_type word = *(col.to_text(0));
		int len_ = *( col.to_int32(1)) ;

		DBJ_PRINT( "\n\t[%3zu]\tword: %32s,\t%s: %12d",
			row_id, word.data(), col.name(1), len_);

		// if all ok, make sure to return always this  sqlite3 macro
		return SQLITE_OK;
	}

	/*
	Test Unit registration
	*/
	TU_REGISTER([] {

		sql::status_type status;
		sql::database const& database = demo_db(status);
		DBJ_RETURN_ON_ERROR( status );

		// register the two udf's required
		// string names of udf's must match the SQL they are part of
		// 	"SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
		// always returning the sql::status_type on error
		// obviuosly macro afficionados are welcome here
		DBJ_RETURN_ON_ERROR(
			sql::register_dbj_udf
			 < palindrome /* function we register */	>(
			 database,  /* to which database */
			 "palindrome" /* how we want it called when used from SQL */
			)
		);

		DBJ_RETURN_ON_ERROR(sql::register_dbj_udf<strlen_udf>(database, "strlen"));

	// execute the query using the 
	// standard result processing calback 
	// "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
	// that SQL will need 'strlen' and 'palindrome' UDF's
	// they are not available in SQLITE3 

		auto SQL = "SELECT Name, strlen(Name) FROM entries WHERE ( 1 == palindrome(Name) )";
		DBJ_RETURN_ON_ERROR(
			database.query(SQL, sql::universal_callback  /* generic dbj::sql result_handler */)
		);

		});
} // namespace dbj_easy_udfs_sample 
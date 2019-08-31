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
	using buffer = typename dbj::nanolib::v_buffer;
	using buffer_type = typename buffer::buffer_type;

	// DB_FILE_PATH setup
	// current default:
	// setup of constexpr inline auto DB_FILE_PATH = "d:\\dictionary.db"sv;
#include "../db_file_path.inc"

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
	/* use case:
	   solve the following query using dbj++sql easy edf feature

	   notice the two udf's required
	   int palindrome( const char *) and int strlen( const char *)
	   they are not available in SQLITE3 SQL as inbuilt functions
	*/
	constexpr inline auto QRY_WITH_UDF
		= "SELECT word, strlen(word) FROM entries WHERE ( 1 == palindrome(word) );";
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
	extern "C" static void palindrome(
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

		// can not use auto here
		// dbj::sql::transformer in action
		buffer_type word_ = args_(0);

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
	extern "C" static void strlen_udf(
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
		sql::database const& database,
		char const* query_ = QRY_WITH_UDF
	) noexcept
	{
		_ASSERTE(query_);
		DBJ_FPRINTF(stdout, "\n\nTEST UDF with the  query: '%s'\n\n", query_);
		return database.query(query_, sql::universal_callback  /*dbj_udfs_result_handler*/);
		// return the status_type
	} // test_udf

	#define CHECK_RETURN if (status) { DBJ_FPRINTF(stdout, "\n\n%s\n\n", (char const*)status); return; }
	/*
	Test Unit registration
	*/
	TU_REGISTER([] {

		status_type status;

		// this is called only on the firs call
		auto initor = [](status_type& stat_)
			-> const sql::database &
		{
			static sql::database db(":memory:", stat_);
			if (stat_) return db; // return on error state
			stat_ = db.exec(DEMO_DB_CREATE_SQL);
			return db;
		};
		// here we keep the single sql::database type instance
		static  sql::database const& database = initor(status);

		CHECK_RETURN;

			// register the two udf's required
			// string names of udf's must match the SQL they are part of
			// 	"SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
			// always returning the status_type on error
			// obviuosly macro afficionados are welcome here
			status = sql::register_dbj_udf<
				palindrome /* function we register */
			>(
				database,  /* to which database */
				"palindrome" /* how we want it called when used from SQL */
				);
			CHECK_RETURN;// return on error status

			status.clear();
			status = sql::register_dbj_udf<strlen_udf>(database, "strlen");
			CHECK_RETURN; // return on error status

			// no error thus proceed
			// just see the :memory: demo db 
			status = test_udf(database, "SELECT * FROM entries;");
			if (status) DBJ_FPRINTF(stdout, "\n\n%s\n\n", (char const*)status);

			// execute the query using the 
			// standard result processing calback 
			// "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
			// that SQL will need 'strlen' and 'palindrome' UDF's
			// they are not available by default
			status = test_udf(database);
			if (status) DBJ_FPRINTF(stdout, "\n\n%s\n\n", (char const*)status);

		});
#undef CHECK_RETURN
} // namespace dbj_easy_udfs_sample 
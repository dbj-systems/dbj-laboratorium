#include "pch.h"
#define DBJ_DB_TESTING
#include "..\dbj++sql\dbj++sql.h"

/* 
   NOTE: we do not need sqlite3 header at all
   thus we do not need the following line:

   using namespace ::sqlite;

   the whole sqlite3 C API is encapsulated and hidden
   (beside the unfortunate macros)
*/

using namespace ::std;
using namespace ::std::string_view_literals ;
using namespace ::dbj::db;

/* one table "words", with one text column "word" */
constexpr inline auto db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"sv;

/* use case: solve executin of the following query using dbjsqlite 3 */
constexpr inline auto query_with_udfs
= "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"sv;

constexpr inline auto show_the_query_result = false;

	/* the palindrome udf */
	inline void palindrome(
		const dbj_sql_udf_value  & value_,
		const dbj_sql_udf_retval & result_
	)
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

		std::string word_ = value_(0);
		int result = is_pal(word_.c_str());
		result_(result);
	}

	/* the strlen udf */
	inline void strlen(
		const dbj_sql_udf_value  & value_,
		const dbj_sql_udf_retval & result_
	)
	{
		std::string word_ = value_(0);
		int result = (int)(word_.size());
		result_(result);
	}

	/* result processor aka callback */
	static int dbj_udfs_result_handler(
		[[maybe_unused]] const size_t row_id,
		[[maybe_unused]] const std::vector<std::string> & col_names,
		[[maybe_unused]] const dbj::db::value_decoder & val_user
	)
	{
		if constexpr (show_the_query_result) {
			/* this is deliberately verbose code */
			std::string word = val_user(0);
			int len_ = val_user(1);
			dbj::console::prinf("\n\t[%3d]\tword: %32s,\tlength: %12d", row_id, word, len_);
		}
		// make sure to return this
		return SQLITE_OK;
	}

	/* called from the test unit */
	void test_udf (
		std::string_view query_	= query_with_udfs
	)
	{
		try	{
			dbj::console::prinf("\nthe query:\n%s\nthe result:\n", query_ );
			// assure the database presence
			dbj::db::database db(db_file);
			// register the udf's required
			register_dbj_udf<palindrome>( db, "palindrome");
			register_dbj_udf<strlen>( db, "strlen");

			db.query(
				query_.data(),
				dbj_udfs_result_handler
			);
		}
		catch (dbj::db::sql_exception const & e)
		{
			dbj::console::prinf(
			"\ndbj::db exception\n code:%3d, message:%64s ", e.code, e.message.c_str()
			);
		}
	} // test_udf


	DBJ_TEST_UNIT(dbj_sql_lite_udf)
	{
		using dbj::console::prinf;
		auto test = [&]( auto fun_ ) {
			prinf("\n%s\nMeasurement start", dbj::LINE());
			auto rezult = fun_();
			prinf("\nMeasurement end\t%s", rezult.c_str());
		};
		
		using namespace dbj::kalends;

		test([&] {  return measure             ([&] { test_udf(); }); });
		test([&] {  return microseconds_measure([&] { test_udf(); }); });
		test([&] {  return miliseconds_measure ([&] { test_udf(); }); });
		test([&] {  return seconds_measure     ([&] { test_udf(); }); });
	}

namespace anyspace {
	/*
	once per each row
	*/
	int example_callback(
		const size_t row_id,
		[[maybe_unused]] const std::vector<std::string> & col_names,
		const dbj::db::value_decoder & val_user
	)
	{
		using dbj::console::print;
		// 'automagic' transform to std::string
		// of the column 0 value for this row
		std::string   word_ = val_user(0);
		print("\n\t", row_id, "\t", word_);

		// all these should provoke exception
		// TODO: but they don't -- currently
		long   DBJ_MAYBE(number_) = val_user(0);
		double DBJ_MAYBE(real_) = val_user(0);
		return SQLITE_OK;
	}

	DBJ_TEST_UNIT(dbj_sql_lite)
	{
		dbj_db_test_::test_insert();
		dbj::console::print("\ndbj_sqlite_callback\n");
		dbj_db_test_::test_select(example_callback);
		dbj::console::print("\ndbj_sqlite_statement_user\n");
		dbj_db_test_::test_statement_using(example_callback);
	}
} // nspace
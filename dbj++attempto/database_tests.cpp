#include "pch.h"

#define DBJ_DB_TESTING
#include "..\dbj++sql\dbj++sql.h"
namespace {
#if 0
	// using callback_type = int(*)(
	// void * , int , char ** , char ** );
	// NOTE! carefull with large data sets, 
	// as this is called once per row
	int dbj_sqlite_callback(
		void *  a_param[[maybe_unused]],
		int argc, char **  argv,
		char ** column
	)
	{
		using dbj::console::print;
		// print the row
		print("\n");
		// we need to know the structure of the row 
		for (int i = 0; i < argc; i++)
			print("\t", i, ": ", column[i], " --> [", argv[i], "]");
		return 0;
	}
#endif
	/*
	once per each row
	*/
	int example_callback(
		const size_t row_id,
		/* this is giving us column count and column names */
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
#undef DBJ_DB_TESTING

namespace
{
	using namespace std;

	extern "C" inline bool is_pal(const char* str) {
		char* s = (char*)str;
		char* e = (char*)str;
		// pomeri e na str kraj 
		// tj na '\0'
		while (*e) e++;
		// vrati ga na zadnje slovo 
		--e;
		// imamo dva setacha
		while (s < e) {
			// razliciti sadrzaji s'leva
			// i s'desna 
			// dakle ne moze biti palindrom
			if (*s != *e) return false;
			// levi napreduje u desno
			++s;
			// desni napreduje u levo
			--e;
			// *moguca* optimizacija je 
			// da je desni levo od levog
			if (e < s) break;
			// prekidamo petlju 
			// rezultat je true
		}
		return true;
	}

#pragma region sqlite udf-s

	using namespace ::sqlite;
	/* this is the UDF */
	static void palindrome(
		sqlite3_context *context, int argc, sqlite3_value **argv)
	{
		sqlite3_result_null(context);
		if (sqlite3_value_type(argv[0]) != SQLITE_TEXT) return;
		if (argc < 1) return;

		char *text = (char*)sqlite3_value_text(argv[0]);
		_ASSERTE(text);
		size_t text_length = sqlite3_value_bytes(argv[0]);
		_ASSERTE(text_length > 0);
		std::string word_{ text, text_length };

		static int result = 0; // aka 'false'
		result = is_pal(word_.c_str());
		sqlite3_result_int(context, result);
		return;
	}

	/*
	once per each row
	*/
	static int dbj_sqlite_result_row_callback(
		[[maybe_unused]] const size_t row_id,
		[[maybe_unused]] const std::vector<std::string> & col_names,
		[[maybe_unused]] const dbj::db::value_decoder & val_user
	)
	{
#if 1
		return SQLITE_OK;
#else
		using dbj::console::print;
		// 'automagic' transform to std::string
		// of the column 0 value for this row
		size_t DBJ_MAYBE(col_count) = col_names.size();
		std::string   word_ = val_user(0);
		// int  is_palindrome_ = val_user(1);
		print("\n\t", row_id, "\t", word_);
		// print("\n\t", row_id, "\t", word_, "\t\t is palindrome: ", (is_palindrome_ ? "true" : "false")
		return SQLITE_OK;
#endif
	}
	void test_udf
	(
		std::string_view query_
		= "SELECT word, palindrome(word) FROM words WHERE word LIKE('bb%')"
	)
	{
		using dbj::console::print;
		constexpr auto db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"sv;

		try
		{
			dbj::db::database db(db_file);
			db.register_user_defined_function("palindrome", palindrome);
			db.query_result(
				query_.data(),
				dbj_sqlite_result_row_callback
			);
		}
		catch (dbj::db::sql_exception const & e)
		{
			print(L"dbj::db exception");
			print("\n code:", e.code, ", message: ", e.message.c_str());
		}
	}
#pragma endregion

	DBJ_TEST_UNIT(dbj_sql_lite_udf)
	{
		using dbj::console::prinf;
		auto test = [&]( auto fun_ ) {
			prinf("\n%s\nMeasurement start", dbj::LINE());
			auto rezult = fun_();
			prinf("\nMeasurement end\t%s", rezult.c_str());
		};
		
		constexpr static auto Q
			= "SELECT word FROM words WHERE 1 == palindrome(word)"sv;

		using namespace dbj::kalends;

		test([&] {  return measure             ([&] { test_udf(Q); }); });
		test([&] {  return microseconds_measure([&] { test_udf(Q); }); });
		test([&] {  return miliseconds_measure ([&] { test_udf(Q); }); });
		test([&] {  return seconds_measure     ([&] { test_udf(Q); }); });
	}
}

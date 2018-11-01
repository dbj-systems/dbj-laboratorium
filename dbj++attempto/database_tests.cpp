#include "pch.h"

#include "../dbj++sql/dbj++sql.h"

namespace
{
	using namespace std;

	extern "C" inline bool petar_pal(const char* str)
	{
		char* a = (char*)str,
			*p = a,
			*q = p;
		int n = strlen(str);
		for (p = a, q = a + n - 1;
			p < q;
			p++, q--
			)
		{
			if (*p != *q) return false;
		}
		return true;
	}

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

	inline void test_palindroma(const char * word_ = "012345678909876543210")
	{
		_ASSERTE(   is_pal("ANA"));
		_ASSERTE(   is_pal(word_));
		_ASSERTE(petar_pal("ANA"));
		_ASSERTE(petar_pal(word_));
	}

#pragma region sqlite udf-s

	/* this is the UDF */
	static void palindrome(sqlite3_context *context, int argc, sqlite3_value **argv)
	{
		sqlite3_result_null(context);
		if (sqlite3_value_type(argv[0]) != SQLITE_TEXT) return;
		if (argc < 1) return;
			
			char *text = (char*)sqlite3_value_text(argv[0]);
			_ASSERTE(text);
			size_t text_length = sqlite3_value_bytes(argv[0]);
			_ASSERTE(text_length > 0);
			std::string word_{ text, text_length };

		static int result = 0 ; // aka 'false'
		result = is_pal(word_.c_str());
		sqlite3_result_int(context, result);
		return;
	}

	/*
	once per each row
	*/
	static int dbj_sqlite_result_row_callback(
		const size_t row_id,
		[[maybe_unused]] const std::vector<std::string> & col_names,
		const dbj::sqlite::value_decoder & val_user
	)
	{
		using dbj::console::print;
		// 'automagic' transform to std::string
		// of the column 0 value for this row
		size_t DBJ_MAYBE(col_count) = col_names.size();
		std::string   word_ = val_user(0);
		// int  is_palindrome_ = val_user(1);
		print("\n\t", row_id, "\t", word_);
		// print("\n\t", row_id, "\t", word_, "\t\t is palindrome: ", (is_palindrome_ ? "true" : "false")
		return SQLITE_OK;
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
			dbj::sqlite::database db(db_file);
			db.register_user_defined_function("palindrome", palindrome);
			db.execute_with_statement(
				query_.data(),
				dbj_sqlite_result_row_callback 
			);
		}
		catch (dbj::sqlite::sql_exception const & e)
		{
			print(L"dbj::sqlite exception");
			print("\n code:", e.code, ", message: ",e.message.c_str());
		}
	}
#pragma endregion
} // namespace

DBJ_TEST_UNIT(dbj_sql_lite_udf) {
	
	constexpr auto Q = "SELECT word FROM words WHERE 1 == palindrome(word)"sv;

	using dbj::console::print;
	auto timer = dbj::kalends::create_timer(dbj::kalends::timer_kind::modern);
	print("\nTimer start\n", dbj::LINE());
	timer.start();
	test_udf(Q);
	print("\n", dbj::LINE() ,"\nTimer end\t",
	dbj::kalends::to_desired_unit<dbj::kalends::MilliSeconds>( timer.elapsed() ).count(),
		" milli seconds"
	);
}
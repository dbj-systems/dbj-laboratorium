#include "pch.h"

#include "../dbj++sql/dbj++sql.h"

namespace
{
	using namespace std;

	struct qd_timer final {

		using Clock = std::chrono::steady_clock;
		using time_point = std::chrono::time_point<Clock>;
		using nano_seconds = std::chrono::nanoseconds;

		time_point start = Clock::now();
		nano_seconds diff{};

		std::string elapsed() {
			time_point end_ = Clock::now();
			diff = end_ - start;
			return std::to_string(diff.count()) + " nanoseconds ";
		}

		std::string micro() {
			// A floating point microseconds type
			using fp_microseconds =
				std::chrono::duration<double, std::chrono::microseconds::period>;
			return std::to_string(fp_microseconds(diff).count()) + " microseconds ";
		}

		std::string mili() {
			// A floating point milliseconds type
			using fp_milliseconds =
				std::chrono::duration<double, std::chrono::milliseconds::period>;
			return std::to_string(fp_milliseconds(diff).count()) + " milliseconds ";
		}

		std::string sec() {
			auto mv = std::chrono::duration_cast<std::chrono::seconds>(diff);
			return std::to_string(mv.count()) + " seconds ";
			// A floating point seconds type
			using fp_seconds =
				std::chrono::duration<double, std::chrono::milliseconds::period>;
			return std::to_string(fp_seconds(diff).count()) + " seconds ";
		}
	};

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
		_ASSERTE(is_pal("ANA"));
		_ASSERTE(is_pal(word_));
		_ASSERTE(petar_pal("ANA"));
		_ASSERTE(petar_pal(word_));
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
			db.execute_with_statement(
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

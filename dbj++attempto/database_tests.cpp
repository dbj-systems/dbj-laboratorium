#include "pch.h"
#define DBJ_DB_TESTING
#include "..\dbj++sql\dbj++sql.h"

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

	using namespace std;
	using namespace ::sqlite;

	struct sqlite3_value_provider final
	{
		/* curently BLOB's are unhandled, they are to be implemented as vector<unsigned char> */
		struct transformer final
		{
			/* if user needs  float, the user will handle that best */
			operator double() const noexcept {
				if (sqlite3_value_type(argv[col_index_]) != SQLITE_FLOAT)
				{
				}
				return sqlite3_value_double(argv[col_index_]);
			}
			operator long() const noexcept {
				if (sqlite3_value_type(argv[col_index_]) != SQLITE_INTEGER)
				{
				}
				return sqlite3_value_int(argv[col_index_]);
			}
			operator long long() const noexcept 
			{
				if (sqlite3_value_type(argv[col_index_]) != SQLITE_INTEGER) 
				{
				}
				return sqlite3_value_int64 (argv[col_index_]);
			}
			operator std::string() const noexcept 
			{
				if (sqlite3_value_type(argv[col_index_]) != SQLITE_TEXT) {
				}
				char *text = (char*)sqlite3_value_text(argv[col_index_]);
				_ASSERTE(text);
				size_t text_length = sqlite3_value_bytes(argv[col_index_]);
				_ASSERTE(text_length > 0);
				return { text, text_length };
			}

			mutable sqlite3_value **argv{};
			mutable size_t			 col_index_;
		};

		/*	return the transformer for a column	*/
		sqlite3_value_provider::transformer 
			operator ()(size_t col_idx) const noexcept
		{
			_ASSERTE(this->argv_);
			return transformer{
				this->argv_, col_idx
			};
		}
		// --------------------------------------
		mutable sqlite3_value **argv_{};
	}; // sqlite3_value_provider

	struct sqlite3_result_provider final {
		
		mutable sqlite3_context *context_;

		// this will cause the sqlite3 to throw the exeception from
		// the udf using mechanism
		void return_error (const std::string & msg_) const noexcept
		{ sqlite3_result_error(context_, msg_.c_str(), msg_.size() ); }

		// sink the result using the appropriate sqlite3 function

		void  operator () (const std::string & value_) const noexcept
		{
			sqlite3_result_text(context_, value_.c_str(), value_.size(), nullptr); 
		}

		void  operator () (std::string_view value_) const noexcept
		{
			sqlite3_result_text(context_, value_.data(), value_.size(), nullptr); 
		}

		void operator () (double value_) const noexcept
		{
			sqlite3_result_double(context_, value_); 
		}

		void operator () (int value_) const noexcept
		{
			sqlite3_result_int(context_, value_); 
		}

		void operator () (long value_) const noexcept
		{
			sqlite3_result_int(context_, value_); 
		}

		void operator () (long long value_) const noexcept
		{
			sqlite3_result_int64(context_, value_);
		}

		void operator () ( nullptr_t ) const noexcept
		{
			sqlite3_result_null(context_); 
		}
	}; // sqlite3_result_provider

	using dbj_sql_udf_type = 
		void (*) (const sqlite3_value_provider  & , const sqlite3_result_provider & );


	using sqlite3_udf_type = void(__cdecl *)
		(sqlite3_context *context, int argc, sqlite3_value **argv);

	template<dbj_sql_udf_type udf_>
	struct udf_holder final
	{
		static void function
		(sqlite3_context *context,
			int argc,
			sqlite3_value **argv)
		{
			(void)noexcept(argc); // unused for now
			_ASSERTE(context);
			_ASSERTE(argv);
			sqlite3_value_provider  values_{ argv };
			sqlite3_result_provider result_{ context };
			udf_(values_, result_);
		}
	};

	/* the dbj UDF required */
	inline void palindrome(
		const sqlite3_value_provider  & value_,
		const sqlite3_result_provider & result_
	)
	{
		std::string word_ = value_(0);
		int result = is_pal(word_.c_str());
		result_(result);
	}

	/* second dbj UDF required */
	inline void strlen(
		const sqlite3_value_provider  & value_,
		const sqlite3_result_provider & result_
	)
	{
		std::string word_ = value_(0);
		int result = (int)(word_.size());
		result_(result);
	}

	/* per each row */
	static int dbj_sqlite_result_row_callback(
		[[maybe_unused]] const size_t row_id,
		[[maybe_unused]] const std::vector<std::string> & col_names,
		[[maybe_unused]] const dbj::db::value_decoder & val_user
	)
	{
		std::string word = val_user(0);
		int len_ = val_user(1);
		dbj::console::print( "\n[", row_id , "] word: ", word, ", length: ", len_ );
		return SQLITE_OK;
	}

	template<dbj_sql_udf_type dbj_udf_>
	void register_dbj_udf (
		const dbj::db::database & db,
		const char * dbj_udf_name_
		)
	{
		using udf_container_type = udf_holder<dbj_udf_>;
		sqlite3_udf_type udf_ = &udf_container_type::function;
		db.register_user_defined_function(dbj_udf_name_, udf_);
	};

	void test_udf (
		std::string_view query_
		= "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"
	)
	{
		constexpr auto db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"sv;
		try	{
			dbj::console::print("\n\nquery:", query_ , "\nthe result:\n");
			dbj::db::database db(db_file);
			
			register_dbj_udf<palindrome>( db, "palindrome");
			register_dbj_udf<strlen>( db, "strlen");

			db.query(
				query_.data(),
				dbj_sqlite_result_row_callback
			);
		}
		catch (dbj::db::sql_exception const & e)
		{
			dbj::console::print(
			"\ndbj::db exception\n code:", e.code, ", message: ", e.message.c_str()
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
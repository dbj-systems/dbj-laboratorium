#pragma once

#include "../dbj++sql.h"

namespace dbj_easy_udfs_sample {
	/*
	   the whole sqlite3 C API is encapsulated and hidden
	   using namespace ::sqlite;
	   (beside the unfortunate macros)
	*/

	using namespace ::std;
	using namespace ::std::string_view_literals;
	using namespace ::dbj::db;
	using namespace ::dbj::db::err ;

	/* one table "words", with one text column "word" */
	constexpr inline auto DB_FILE = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"sv;

	/* use case: solve executin of the following query using dbjsqlite 3 */
	constexpr inline auto QRY_WITH_UDF
		= "SELECT word, strlen(word) FROM words WHERE (1 == palindrome(word))"sv;
	/* the palindrome udf */
	inline void palindrome(
		const udf_argument  & value_,
		const udf_retval & result_
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
		const udf_argument  & argument_,
		const udf_retval & result_
	)
	{
		std::string word_ = argument_(0);
		int result = (int)(word_.size());
		result_(result);
	}

	constexpr inline auto show_the_query_result = true;

	/* result processor aka callback */
	static int dbj_udfs_result_handler(
		const size_t row_id,
		const dbj::db::row_descriptor & row
	)
	{
		if constexpr (show_the_query_result) {
			/* this is deliberately verbose code */
			std::string word = row(0);
			int len_ = row(1);
			::wprintf(L"\n\t[%3d]\tword: %32S,\tlength: %12d", 
				static_cast<int>(row_id), word.c_str(), len_);
		}
		// make sure to return this
		return SQLITE_OK;
	}

	/* called from the test unit */
	[[nodiscard]] error_code test_udf(
		std::string_view query_ = QRY_WITH_UDF
	) noexcept 
	{
			::wprintf(L"\nthe query:\t%S\nthe result:\n", query_.data());
			// assure the database presence
			error_code ec;
			database db(DB_FILE, ec); if (ec) return ec;
			// register the udf's required
			register_dbj_udf<palindrome>(db, "palindrome");
			register_dbj_udf<strlen>(db, "strlen");

			return db.query(query_.data(),dbj_udfs_result_handler); 
	} // test_udf

} // namespace dbj_easy_udfs_sample 

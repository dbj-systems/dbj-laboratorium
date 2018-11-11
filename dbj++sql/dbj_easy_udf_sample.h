#pragma once
/*
Copyright 2018 by dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "dbj++sql.h"

namespace dbj_easy_udfs_sample {
	/*
	   NOTE: we do not need sqlite3 header at all
	   thus we do not need the following line:

	   using namespace ::sqlite;

	   the whole sqlite3 C API is encapsulated and hidden
	   (beside the unfortunate macros)
	*/

	using namespace ::std;
	using namespace ::std::string_view_literals;
	using namespace ::dbj::db;

	/* one table "words", with one text column "word" */
	constexpr inline auto db_file = "C:\\dbj\\DATABASES\\EN_DICTIONARY.db"sv;

	/* use case: solve executin of the following query using dbjsqlite 3 */
	constexpr inline auto query_with_udfs
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

	constexpr inline auto show_the_query_result = false;

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
			::printf("\n\t[%3d]\tword: %32s,\tlength: %12d", row_id, word.c_str(), len_);
		}
		// make sure to return this
		return SQLITE_OK;
	}

	/* called from the test unit */
	void test_udf(
		std::string_view query_ = query_with_udfs
	)
	{
		try {
			::printf("\nthe query:\n%s\nthe result:\n", query_.data());
			// assure the database presence
			dbj::db::database db(db_file);
			// register the udf's required
			register_dbj_udf<palindrome>(db, "palindrome");
			register_dbj_udf<strlen>(db, "strlen");

			db.query(
				query_.data(),
				dbj_udfs_result_handler
			);
		}
		catch (dbj::db::sql_exception const & e)
		{
			::printf(
				"\ndbj::db exception\n code:%3d, message:%s ", e.code, e.message.c_str()
			);
		}
	} // test_udf

} // namespace dbj_easy_udfs_sample 

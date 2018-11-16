#include "pch.h"
#define DBJ_DB_TESTING
#include "..\dbj++sql\core_tests.h"
#include "..\dbj++sql\dbj_easy_udf_sample.h"

DBJ_TEST_UNIT(dbj_sql_lite_udf)
{
	using dbj::console::prinf;
	using namespace dbj::kalends;
	using namespace dbj_easy_udfs_sample;

	auto test = [&]( auto fun_ ) {
		prinf("\n%s\nMeasurement start", dbj::LINE());
		auto rezult = fun_();
		prinf("\nMeasurement end\t%s", rezult.c_str());
	};

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
		dbj_db_test_::test_select();
		dbj::console::print("\ndbj_sqlite_statement_user\n");
		dbj_db_test_::test_statement_using(example_callback);
	}
} // nspace
#include "pch.h"

void test_dbj_sql_lite_udf()
{
	using namespace ::dbj::log; 
	namespace k = dbj::kalends;
	namespace d = dbj_easy_udfs_sample;
	using ::dbj::console::print;

	auto test = [&](auto fun_) 
	{
		DBJ_LOG_INF("Measurement start, %s", 
			DBJ_ERR_PROMPT("test_dbj_sql_lite_udf()"));
		;  std::string rezult = fun_();
		DBJ_LOG_INF(
			"Measurement end, result: %s", DBJ_ERR_PROMPT(rezult.data())
		);
	};

	test([&] {  return k::miliseconds_measure([&] {
		// DBJ_LOG_STD_ERR(d::test_udf(),"dbj_easy_udfs_sample::test_udf() has failed");
	}); });

	/*
		test([&] {  return measure             ([&] { test_udf(); }); });
		test([&] {  return microseconds_measure([&] { test_udf(); }); });
		test([&] {  return seconds_measure     ([&] { test_udf(); }); });
	*/
}

/*
once per each row
*/
int example_callback(
	const size_t row_id,
	const dbj::db::row_descriptor & row_
)
{
	using dbj::console::print;
	// 'automagic' transform to std::string
	// of the column 0 value for this row
	dbj::chr_buf::yanb   word_ = row_(0);
	print("\n\t", row_id, "\t", word_);

	// all these should provoke exception
	// TODO: but they don't -- currently
	long   DBJ_MAYBE(number_) = row_(0);
	double DBJ_MAYBE(real_) = row_(0);
	return SQLITE_OK;
}

void test_dbj_sql_lite()
{
	using dbj::console::print;
	std::error_code err;

	DBJ_LOG_STD_ERR(dbj_db_test_::test_insert(),"dbj_db_test_::test_insert()");

	err.clear(); 
	DBJ_LOG_STD_ERR( dbj_db_test_::test_table_info(),"dbj_db_test_::test_table_info()");

	print("\n\ndbj_sqlite_callback\n");
	err.clear(); 
	DBJ_LOG_STD_ERR( dbj_db_test_::test_select(), "dbj_db_test_::test_select()");

	print("\n\ndbj_sqlite_statement_user\n");
	err.clear(); 
	DBJ_LOG_STD_ERR(dbj_db_test_::test_statement_using(example_callback),"dbj_db_test_::test_statement_using(example_callback)");
}




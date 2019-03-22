#include "pch.h"

#include "..\dbj++sql\test\core_tests.h"
#include "..\dbj++sql\test\dbj_easy_udf_sample.h"

void test_dbj_sql_lite_udf()
{
	namespace k = dbj::kalends;
	namespace d = dbj_easy_udfs_sample;
	using ::dbj::console::print;

	auto test = [&](auto fun_) {
		using ::dbj::console::print;
		print("\n", dbj::LINE(), "%s\nMeasurement start");
		auto rezult = fun_();
		print("\n\n\tMeasurement result: ", rezult);
	};
	/*
		test([&] {  return measure             ([&] { test_udf(); }); });
		test([&] {  return microseconds_measure([&] { test_udf(); }); });
		*/
	test([&] {  return k::miliseconds_measure([&] {
		auto err = d::test_udf();
		if (err)
			print("\n", err);
	}); });
	/*
		test([&] {  return seconds_measure     ([&] { test_udf(); }); });
	*/
	system("@echo.");
	system("@echo.");
	system("@pause");

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
	dbj::buf::yanb   word_ = row_(0);
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

	std::error_code err = dbj_db_test_::test_insert();
	if (err) print("\n", err);

	err.clear(); err = dbj_db_test_::test_table_info();
	if (err) print("\n", err);

	print("\n\ndbj_sqlite_callback\n");
	err.clear(); err = dbj_db_test_::test_select();
	if (err) print("\n", err);

	print("\n\ndbj_sqlite_statement_user\n");
	err.clear(); err = dbj_db_test_::test_statement_using(example_callback);
	if (err) print("\n", err);

	system("@echo.");
	system("@echo.");
	system("@pause");
}




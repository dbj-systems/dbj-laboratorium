#include "../pch.h"
#include "core_tests.h"

namespace two_tests
{
	namespace sql = ::dbj::sql;

	using status_type = typename dbj::sql::dbj_db_status_type;
	using buffer = typename dbj::sql::v_buffer;
	using buffer_type = typename dbj::sql::v_buffer::buffer_type;

	void test_dbj_sql_lite_udf()
	{
		namespace sql = ::dbj::sql;

		using status_type = typename dbj::sql::dbj_db_status_type;
		using buffer = typename dbj::sql::v_buffer;
		using buffer_type = typename dbj::sql::v_buffer::buffer_type;

		using namespace ::dbj::log;
		namespace k = dbj::kalends;
		// namespace d = dbj_easy_udfs_sample;
		using ::dbj::console::print;

		auto test = [&](auto fun_)
		{
			DBJ_FPRINTF( stdout,"Measurement start, %s",
				DBJ_ERR_PROMPT("test_dbj_sql_lite_udf()"));
			;  std::string rezult = fun_();
			DBJ_FPRINTF( stdout,
				"Measurement end, result: %s", DBJ_ERR_PROMPT(rezult.data())
			);
		};

		test([&] {  return k::miliseconds_measure([&] {
			// DBJ_FPRINTF( stdout,d::test_udf(),"dbj_easy_udfs_sample::test_udf() has failed");
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
		const dbj::sql::row_descriptor& row_
	)
	{
		// 'automagic' transform to the buffer type
		// of the column 0 value for this row
		buffer_type  word_ = row_(0);
		DBJ_FPRINTF(stdout, "\n\t ROW (%zd) : \t %s", row_id, word_.data() );

		// all these should provoke exception
		// TODO: but they don't -- currently
		long   DBJ_MAYBE(number_) = row_(0);
		double DBJ_MAYBE(real_) = row_(0);
		return SQLITE_OK;
	}

	void test_dbj_sql_lite()
	{
		::dbj::sql::dbj_db_status_type  status_ ;

		status_ = dbj_db_test_::test_insert();
		DBJ_FPRINTF(stdout, "\n Status : \t %s", status_.to_buffer(status_).data());

		status_.clear();
		status_ = dbj_db_test_::test_table_info();
		DBJ_FPRINTF(stdout, "\n Status : \t %s", status_.to_buffer( status_ ).data( ));

		status_.clear();
		status_ = dbj_db_test_::test_select();
		DBJ_FPRINTF(stdout, "\n Status : \t %s", status_.to_buffer(status_).data());

		status_.clear();
		status_ = dbj_db_test_::test_statement_using(example_callback);
		DBJ_FPRINTF(stdout, "\n Status : \t %s", status_.to_buffer(status_).data());

		/*
		NOTE: above we just print-and-proceed 
		usually callers will use it to develop another logic
		id there is an error they wish to process and act 
		on the status_
		*/
	}



} // dbj_db_test_
#define DBJ_WMAIN_USED
#include "pch.h"

#ifndef DBJ_WMAIN_USED
#pragma message( "\n\n\n" __FILE__)
#pragma message( "\nDBJ_WMAIN_USED has to be defined \n\n\n" )
#error DBJ_WMAIN_USED undefined?
#endif

// used in headers bellow
namespace sql = ::dbj::sql;
// three important types used everywhere 
using status_type = typename dbj::sql::dbj_db_status_type;
using buffer = typename dbj::sql::v_buffer;
using buffer_type = typename dbj::sql::v_buffer::buffer_type;


#include "tests/two_tests.h"
#include "tests/dbj_easy_udf_sample.h"

#pragma warning( push )
#pragma warning( disable: 4100 )
// https://msdn.microsoft.com/en-us/library/26kb9fy0.aspx 

/// <summary>
/// called from wmain() form inside dbj++
/// just execute all the registered tests
/// in no particular order
/// </summary>
void dbj_program_start(
	const int argc,
	const wchar_t* argv[],
	const wchar_t* envp[]
)
{
	DBJ_TRACE(L"\n\n%s -- Started\n", argv[0] );

	two_tests::test_dbj_sql_lite();
	two_tests::test_dbj_sql_lite_udf();

	// TODO: make this async
	// dbj::testing::execute(argc, argv, envp);
	DBJ_TRACE(L"\n\n%s -- Finished\n", argv[0]);
}

#pragma warning( pop ) // 4100
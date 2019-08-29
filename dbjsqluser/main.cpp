#include "pch.h"

#ifndef DBJ_WMAIN_USED
#pragma message( "\n\n\n" __FILE__)
#pragma message( "\nDBJ_WMAIN_USED has to be defined \n\n\n" )
#error DBJ_WMAIN_USED undefined?
#endif

#include "tests/core_tests.h"
#include "tests/dbj_easy_udf_sample.h"
#include "tests/more_dbj_db_tests.h"
#include "tests/ini_file_and_syslog_testing.h"


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
	// TODO: make this async
	dbj::testing::execute(argc, argv, envp);
	DBJ_TRACE(L"\n\n%s -- Finished\n", argv[0]);
}

#pragma warning( pop ) // 4100
#include "pch.h"

// check is valstat is here and on the include path
#include <valstat>

#ifndef DBJ_WMAIN_USED
#pragma message( "\n\n\n" __FILE__)
#pragma message( "\nDBJ_WMAIN_USED has to be defined \n\n\n" )
#error DBJ_WMAIN_USED undefined?
#endif

#pragma warning( push )
#pragma warning( disable: 4100 )
// https://msdn.microsoft.com/en-us/library/26kb9fy0.aspx 

/// <summary>
/// just execute all the registered tests
/// in no particular order
/// </summary>
void dbj_program_start(
	const int argc,
	const wchar_t *argv[],
	const wchar_t *envp[]
)
{
	DBJ_TRACE(L"\n\n%s -- Started\n", argv[0]);
	// TODO: make this async
	dbj::testing::execute(argc, argv, envp);
	DBJ_TRACE(L"\n\n%s -- Finished\n", argv[0]);
 }

#pragma warning( pop ) // 4100
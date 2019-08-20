#include "pch.h"

#ifndef DBJ_WMAIN_USED
#error __FILE__
#error DBJ_WMAIN_USED has to be defined 
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
	::dbj::core::trace(L"\n\nDBJ++ATTEMPTO -- ::dbj::console_is_initialized()\n");
	// TODO: make this async
	dbj::testing::execute(argc, argv, envp);
 }

#pragma warning( pop ) // 4100
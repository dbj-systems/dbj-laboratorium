#include "pch.h"

#ifndef DBJ_WMAIN_USED
#error __FILE__
#error DBJ_WMAIN_USED has to be defined 
#endif

//#define DBJ_SYSLOG
//#include <dbj++log/dbj++log.h>

#include "object_store/dbj_object_store.h"
#include "compile_time_shenanigans.h"
#include "constexpr-format-master/constexpr_format_test.h"


#pragma warning( push )
#pragma warning( disable: 4100 )
// https://msdn.microsoft.com/en-us/library/26kb9fy0.aspx 



/// <summary>
/// just execute all the registered tests
/// in no particular order
/// </summary>
void dbj_program_start(
	const	int			argc,
	const	wchar_t *	argv[],
	const	wchar_t *	envp[]
) 
{
	DBJ_TRACE(L"\nDBJ++ATTEMPTO -- dbj++log is initialized");
	DBJ_TRACE(L"\n\n%s -- Started\n", argv[0]);
	// TODO: make this async
	dbj::testing::execute(argc, argv, envp);
	DBJ_TRACE(L"\n\n%s -- Finished\n", argv[0]);
}

#pragma warning( pop ) // 4100
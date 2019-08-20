#include "pch.h"

#ifndef DBJ_WMAIN_USED
#error __FILE__
#error DBJ_WMAIN_USED has to be defined 
#endif

#include <dbj++log/dbj++log.h>

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
	const int argc,
	const wchar_t *argv[],
	const wchar_t *envp[]
) 
{
	::dbj::log::syslog_init();
	::dbj::log::syslog_open("dbj++attempto", ::dbj::log::syslog_open_options::log_perror );
	::dbj::log::syslog_info("syslog connection opened");
	::dbj::core::trace("\nDBJ++ATTEMPTO -- dbj++log is initialized");
	// TODO: make this async
	dbj::testing::execute(argc, argv, envp);
	::dbj::log::syslog_info("tests executions finished in an orderly fashion");
}

#pragma warning( pop ) // 4100
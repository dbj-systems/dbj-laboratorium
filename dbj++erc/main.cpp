// dbj++erc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

//#include "dbj_error_type.h"
//#include "r_and_d.h"
//#include "snippets.h"

#ifdef DBJ_C_LIB_TEST
#include "dbj_c_lib_test.h"
#endif

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
	::dbj::core::trace("\nDBJ++ERR -- ::dbj::console_is_initialized()");
	// TODO: make this async
	dbj::testing::execute(argc, argv, envp);
}

#if 0
#ifdef UNICODE
int wmain(const int argc, const wchar_t *argv[], const wchar_t *envp[])
#else
#error "What could be the [t]reason this is not a UNICODE build?"
#endif
{
	using namespace ::dbj::console;
	try {
		dbj_program_start(argc, argv, envp);
	}
	catch (...) {
		print(
			painter_command::bright_red,
			"\n\n" __FUNCSIG__ "  Unknown exception caught! \n\n",
			painter_command::text_color_reset
		);
		return EXIT_FAILURE;
	}
	return  EXIT_SUCCESS;
}
#endif
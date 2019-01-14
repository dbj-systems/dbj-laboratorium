// dbj++erc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "error_handling/dbj_exception.h"
#include "snippets.h"

#ifdef UNICODE
int wmain(const int argc, const wchar_t *argv[], const wchar_t *envp[])
#else
#error "What could be the [t]reason this is not a UNICODE build?"
#endif
{
	using namespace ::dbj::console;
	try {
	}
	catch (...) {
		print(
			painter_command::bright_red,
			__FUNCSIG__ "  Unknown exception caught! ",
			painter_command::text_color_reset
		);
	}
	return  EXIT_SUCCESS;
}
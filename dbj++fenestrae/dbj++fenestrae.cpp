// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"

#pragma warning( push )
#pragma warning( disable: 4100 )
// https://msdn.microsoft.com/en-us/library/26kb9fy0.aspx 

/// <summary>
/// just execute all the registered tests
/// in no particular order
/// </summary>
static void program_start(
	const int argc,
	const wchar_t *argv[],
	const wchar_t *envp[]
) {
	dbj::testing::execute(argc, argv, envp);
	//	dbj::log::flush();
}

#ifdef UNICODE
int wmain(const int argc, const wchar_t *argv[], const wchar_t *envp[])
#else
#error "What could be the [t]reason this is not UNICODE build?"
int main(int argc, char* argv[], char *envp[])
#endif
{
	try {
		program_start(argc, argv, envp);
	}
	catch (...) {
		using namespace dbj::console;
		dbj::console::print(
			painter_command::bright_red,
			__FUNCSIG__ "  Unknown exception caught! ",
			painter_command::text_color_reset
		);
	}
	return  EXIT_SUCCESS;
}

#pragma warning( pop ) // 4100

#include "pch.h"
#include "dbj_matrix.h"

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
}

static std::wstring app_base_name_;
/*
this works only if and when called from inside the catch block
*/
static void final_exceptions_processor()
{
	using namespace ::dbj::console ;

	print( "\n" , app_base_name_ , " -- Exception caught! -- ", painter_command::bright_red );

	try {
		throw;
	}
	catch (::dbj::Exception & x_) {
		print( "dbj Exception, ",	x_.what() );
	}
	catch (std::system_error & x_) {
		print( "system error: ", x_.code() );
	}
	catch (std::exception & x_) {
		print("std exception: ", x_.what() );
	}
	catch (...) {
		print(" Unknown Exception ");
	}
	print(painter_command::text_color_reset, "\n");
}

#ifdef UNICODE
int wmain(const int argc, const wchar_t *argv[], const wchar_t *envp[])
#else
#error "What could be the [t]reason this is not an UNICODE build?"
int main(int argc, char* argv[], char *envp[])
#endif
{
	try {

		app_base_name_ = argv[0];
		app_base_name_.erase(0,
			1 + app_base_name_.find_last_of(L"\\")
		);

		program_start(argc, argv, envp);
	}
	catch (...) {
		final_exceptions_processor();
	}
	return  EXIT_SUCCESS;
}

#pragma warning( pop ) // 4100
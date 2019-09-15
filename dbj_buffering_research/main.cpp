// dbj_buffering_research.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cstdio>
#include <future>
#include <ctime>
#include <chrono>
#include <map>

#include "dbj--nanolib/dbj++tu.h"

#include "dbj_buffers.h"
#include "shared_pointer_buffer.h"

#ifdef DBJ_JUST_TESTS
#include "dbj_buffer_testing.h"
#endif

#include "dbj_buffer_measurer.h"

#pragma warning( push )
#pragma warning( disable: 4100 )
// https://msdn.microsoft.com/en-us/library/26kb9fy0.aspx 


/*
*/
void start_from_separate_thread(
	const int	argc,
	const char* argv[],
	const char* envp[]
)
{
	dbj::nanolib::set_console_font( L"Lucida Console", 24);

	DBJ_PRINT(u8"\n(c) 2019 by Dušan B. Jovanović -- dbj@dbj.org\n");
	DBJ_PRINT("\n\n%s -- Starting\n", 1 + strrchr(argv[0], '\\'));	
	dbj::tu::catalog.execute();
	DBJ_PRINT("\n\n%s -- Finished\n", 1 + strrchr(argv[0], '\\'));

#ifdef _MSC_VER
	::system("@echo.");
	::system("@echo.");
	::system("@pause");
#endif
}

int main(int argc, char const* argv[], char const* envp[])
{
	auto main_worker = [&]() {
		try {
			start_from_separate_thread(argc, argv, envp);
		}
		catch (...) {
			DBJ_FPRINTF(stderr, "\n\nAn Unknown Exception caught!\n\n");
			::exit(EXIT_FAILURE);
		}

	};

	(void)std::async(std::launch::async, [&] {
		main_worker();
		});

	::exit(EXIT_SUCCESS);
}


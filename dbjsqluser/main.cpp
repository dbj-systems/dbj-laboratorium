/*
(c) 2019 by dbj@dbj.org -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/
*/  
#include "pch.h"

#include "tests/basic_tests.h"
#include "tests/easy_udf_sample.h"
#include "tests/naked_udf.h"

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
	DBJ_PRINT( "\n\n%s -- Started\n", argv[0] );
		dbj::tu::catalog.execute();
	DBJ_PRINT( "\n\n%s -- Finished\n", argv[0]);

#ifdef _DEBUG
#ifdef _MSC_VER
	::system("@echo.");
	::system("@echo.");
	::system("@pause");
#endif
#endif
}

int main(int argc, char const * argv[], char const * envp[])
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

#pragma warning( pop ) // 4100
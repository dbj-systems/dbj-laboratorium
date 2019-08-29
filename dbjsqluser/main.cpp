#include "pch.h"

// used in headers bellow
namespace sql = ::dbj::sql;
// three important types used everywhere 
using status_type	= typename dbj::sql::dbj_db_status_type;
using buffer		= typename dbj::nanolib::v_buffer;
using buffer_type	= typename dbj::nanolib::v_buffer::buffer_type;


#include "tests/two_tests.h"
#include "tests/dbj_easy_udf_sample.h"

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
	DBJ_FPRINTF( stdout, "\n\n%s -- Started\n", argv[0] );
		dbj::tu::catalog.execute();
	DBJ_FPRINTF( stdout, "\n\n%s -- Finished\n", argv[0]);
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
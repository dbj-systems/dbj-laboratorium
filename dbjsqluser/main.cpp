#include "pch.h"

// #include "..\dbj++sql\dbj_log\dbj_log.h"

#define DBJ_LOG_TESTING
#include "..\dbj++log\dbj++log.h"

extern void test_dbj_sql_lite_udf();
extern void test_dbj_sql_lite();

// int wmain( int argc, wchar_t * argv [], wchar_t * envp )
int main( int argc, char * argv [])
{
	dbj_log_test(argc, argv);
	// test_dbj_sql_lite_udf();
	// test_dbj_sql_lite();
}

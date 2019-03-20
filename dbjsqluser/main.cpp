#include "pch.h"

#define DBJ_LOG_TESTING
#include <dbj++log/dbj++log.h>

#define DBJ_INI_READER_TESTING
#include <dbj++ini/ini_reader_sampler.h>

extern void test_dbj_sql_lite_udf();
extern void test_dbj_sql_lite();

// int wmain( int argc, wchar_t * argv [], wchar_t * envp )
int main( int argc, char * argv [])
{

	dbj::ini::ini_file_descriptor ifd = dbj::ini::ini_file();
	char * my_data[] = { (char*)ifd.fullpath.get() };

	test_dbj_ini_reader(1, my_data);
	dbj_log_test(argc, argv);
	// test_dbj_sql_lite_udf();
	// test_dbj_sql_lite();
}

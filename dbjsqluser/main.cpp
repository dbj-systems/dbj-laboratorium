#include "pch.h"

extern void test_dbj_sql_lite_udf();
extern void test_dbj_sql_lite();

int wmain( int argc, wchar_t * argv [], wchar_t * envp )
{
	test_dbj_sql_lite_udf();
	test_dbj_sql_lite();
}

#include "pch.h"

#include "..\dbj++sql\dbj_log\dbj_log.h"

extern void test_dbj_sql_lite_udf();
extern void test_dbj_sql_lite();

int wmain( int argc, wchar_t * argv [], wchar_t * envp )
{
	dbj::db::log::info("info one", "info two");
	dbj::db::log::warning ("warning one", "warning two");
	dbj::db::log::error ("error one", "error two");
	test_dbj_sql_lite_udf();
	test_dbj_sql_lite();
}

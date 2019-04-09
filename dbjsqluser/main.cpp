#include "pch.h"
#include <dbj++/testfwork/dbj_testing_interface.h>
#include <dbj++/numeric/dbj_numeric.h>
#include <dbj++log/dbj++log.h>
#ifdef DBJ_LOG_TESTING
static void log_sampler();
#endif

#ifdef DBJ_INI_READER_TESTING
static void ini_sampler();
#endif


extern void test_dbj_sql_lite_udf();
extern void test_dbj_sql_lite();

// have to do it here so that caller can use it before main ...
static auto log_init = []() 
{
	// namespace galimatias
	using dbj::buf::yanb;
	using ::dbj::core::trace;
	using namespace ::dbj::win32;
	using namespace ::dbj::log;

	yanb basename_ = module_basename();
	syslog_init();
	syslog_open(
		"dbjsqluser", syslog_open_options::log_perror
	);
	syslog_info("syslog connection opened");
	trace("\ndbj++log is initialized");
	return true;
}();

// int wmain( int argc, wchar_t * argv [], wchar_t * envp )
int main( int argc, char * argv [])
{
#ifdef _DEBUG
	DBJ_TEST_ATOM(dbj::num::reverse(12345));
	// overflow must return 0
	DBJ_TEST_ATOM(dbj::num::reverse(INT32_MAX));
	// underflow must return 0
	DBJ_TEST_ATOM(dbj::num::reverse(INT32_MIN));

	// this requires app to have a proper manifest
	DBJ_TEST_ATOM(dbj::win32::is_windows_10());
#endif // _DEBUG

	test_dbj_sql_lite_udf();
	test_dbj_sql_lite();
}


#ifdef DBJ_INI_READER_TESTING
#include <dbj++ini/ini_reader_sampler.h>

static void ini_sampler()
{
	auto test_ini = [](dbj::buf::yanb folder_)
		-> dbj::buf::yanb
	{
		std::string rv(folder_);
		return (rv.append("\\test.ini")).data();
	};

	dbj::ini::ini_file_descriptor ifd = dbj::ini::ini_file();
	dbj::buf::yanb my_data[] = { test_ini(ifd.folder) };
	test_dbj_ini_reader(1, my_data);
}
#endif // DBJ_INI_READER_TESTING

#ifdef DBJ_LOG_TESTING
static void log_sampler()
{
	// namespace galimatias
	using dbj::buf::yanb;
	using namespace ::dbj::win32;
	using namespace ::dbj::log;

	yanb basename_ = module_basename();
	dbj_log_test(0, basename_);
}
#endif

#include "pch.h"

#define DBJ_SYSLOG (1==1)
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
// this is an "self executing" lambda
static auto log_init = []() 
{
	// namespace galimatias
	// almost like cppwinrt ;)
	using dbj::buf::yanb;
	using ::dbj::core::trace;
	using namespace ::dbj::win32;
	using namespace ::dbj::log;

	yanb basename_ = module_basename();
	syslog_init();
	// syslog_open_options::log_perror
	// makes use of local log file
	syslog_open(
		"dbjsqluser" /*, syslog_open_options::log_perror*/
	);
	DBJ_LOG_INF("syslog connection opened from %s", basename_.data() );
	trace("\ndbj++log is initialized");
	return true;
}();

// int wmain( int argc, wchar_t * argv [], wchar_t * envp )
int main( int argc, char * argv [])
{
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

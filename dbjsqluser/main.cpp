#include "pch.h"

#ifdef DBJ_LOG_TESTING
static void log_sampler();
#endif

#ifdef DBJ_INI_READER_TESTING
static void ini_sampler();
#endif


extern void test_dbj_sql_lite_udf();
extern void test_dbj_sql_lite();

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
	auto test_ini = [](dbj::chr_buf::yanb folder_)
		-> dbj::chr_buf::yanb
	{
		std::string rv(folder_);
		return (rv.append("\\test.ini")).data();
	};

	dbj::ini::ini_file_descriptor ifd = dbj::ini::ini_file();
	dbj::chr_buf::yanb my_data[] = { test_ini(ifd.folder) };
	test_dbj_ini_reader(1, my_data);
}
#endif // DBJ_INI_READER_TESTING

#ifdef DBJ_LOG_TESTING
static void log_sampler()
{
	// namespace galimatias
	using dbj::chr_buf::yanb;
	using namespace ::dbj::win32;
	using namespace ::dbj::log;

	yanb basename_ = module_basename();
	dbj_log_test(0, basename_);
}
#endif

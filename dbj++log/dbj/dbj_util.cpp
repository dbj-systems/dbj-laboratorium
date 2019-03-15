#include "dbj_util.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <array>
#include <chrono>
#include <errno.h>
#include <cassert>
#include <filesystem>
#include <stdio.h>

[[noreturn]] void dbj_terror
(char const * msg_, char const * file_, const unsigned line_)
{
	assert(msg_ != nullptr); assert(file_ != nullptr); assert(line_ > 0);
	::fprintf(stderr, "\n\nTerminating ERROR:%s\n\n%s (%d)\n\n", msg_, file_, line_);
#ifdef _DEBUG
#ifdef _MSC_VER
	::system("@echo.");
	::system("@pause");
	::system("@echo.");
#endif
#endif
	exit(EXIT_FAILURE);
}

namespace h = ::std::chrono;

// time stamp size is max 22 + '\0'
char * dbj_time_stamp(
	char const * timestamp_mask_
)
{
	static std::array<char, 32U> buf_arr{ {0} };
	static const size_t buf_len = buf_arr.size();

	buf_arr.fill(0); // zero the buffer

	char * buf = buf_arr.data();

	if (!timestamp_mask_)
		timestamp_mask_ = DBJ_TIME_STAMP_SIMPLE_MASK;

	// Get the current time
	auto now = h::system_clock::now();
	// Format the date/time
	time_t now_tm_t = h::system_clock::to_time_t(now);
	struct tm  local_time_ {};

	errno_t posix_err_code = ::localtime_s(&local_time_, &now_tm_t);
	// leave the result empty if error
	if (posix_err_code > 0) {
		errno = posix_err_code;
		return NULL;
	}

	strftime(buf, buf_len, timestamp_mask_, &local_time_);
	// Get the milliseconds
	int millis = h::time_point_cast<h::milliseconds>(now).time_since_epoch().count() % 100;
	// 
	const auto strlen_buf = std::strlen(buf);
	(void)::sprintf_s(buf + strlen_buf, buf_len - strlen_buf, ".%03d", millis);

	return buf ;
};

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

const char * dbj_get_envvar(char * varname_) 
{
	_ASSERTE(varname_);
	static std::array<char, 256>	bar{ {0} };
	bar.fill(0); // zero the buffer
	::SetLastError(0);
	if (1 > ::GetEnvironmentVariableA(varname_, bar.data(), (DWORD)bar.size()))
	{
		std::error_code  er_ = std::error_code(::GetLastError(), std::system_category());

		DBJ_VERIFY( ! er_.message().c_str() );
	}
	return bar.data() ;
}

/*
*/
std::filesystem::path dbj_program_data_path()
{
	namespace fs = std::filesystem;

	std::string program_data{ dbj_get_envvar((char*)"ProgramData") };

	static fs::path prog_data_path_;
	prog_data_path_ = program_data ;
    return prog_data_path_ ;
}
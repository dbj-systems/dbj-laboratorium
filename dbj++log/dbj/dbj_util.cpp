#include "dbj_util.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <array>
#include <chrono>
#include <errno.h>
#include <cassert>
#include <filesystem>
#include <stdio.h>

char * dbj_basename(char * full_path) {
	char *p = full_path, *pp = 0;
	while ((p = strchr(p + 1, '\\'))) {
		pp = p;
	}
	return (pp ? pp + 1 : p);
}
/*----------------------------------------------------------------------*/
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
/*----------------------------------------------------------------------*/
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

/*
old fashioned timestamp as required by RFC3164
*/
void dbj_timestamp_rfc3164( char (*timestamp_rfc3164)[0xFF], int require_milli_secs ) 
{
	static char const * month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
						"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	SYSTEMTIME stm;
	GetLocalTime(&stm);

	unsigned len = 0;
	/* THIS IS https://tools.ietf.org/html/rfc3164 FORMAT */
	if (require_milli_secs == 0) {
		len = sprintf_s(*timestamp_rfc3164, sizeof(*timestamp_rfc3164),
			"%s %2d %02d:%02d:%02d",
			month[stm.wMonth - 1], stm.wDay, stm.wHour, stm.wMinute, stm.wSecond);
	}
	else {
		len = sprintf_s(*timestamp_rfc3164, sizeof(*timestamp_rfc3164),
			"%s %2d %02d:%02d:%02d.%03d",
			month[stm.wMonth - 1], stm.wDay, stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds);
	}
	_ASSERTE(len);
}

/*
https://stackoverflow.com/a/54491532/10870835
*/
template <typename CHR, typename win32_api>
std::basic_string<CHR> 
string_from_win32_call(win32_api win32_call, unsigned initialSize = 0U)
{
	std::basic_string<CHR> result((initialSize == 0 ? MAX_PATH : initialSize), 0);
	for (;;)
	{
		auto length = win32_call(&result[0], (int)result.length());
		if (length == 0)
		{
			return std::basic_string<CHR>();
		}

		if (length < result.length() - 1)
		{
			result.resize(length);
			result.shrink_to_fit();
			return result;
		}

		const auto rl_ = result.length();
		result.resize( rl_ + rl_ );
	}
}

std::string dbj_module_basename_2(HINSTANCE h_instance) {
	
	std::string module_path 
		= string_from_win32_call<char>([h_instance](char* buffer, int size)
	{
		return GetModuleFileNameA(h_instance, buffer, size);
	});

	_ASSERTE(module_path.empty() == false);

	return { dbj_basename(module_path.data()) };
}
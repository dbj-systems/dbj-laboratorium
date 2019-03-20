
#include "../dbj++ini.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _MSVC_LANG
#if _MSVC_LANG < 201402L
#error "C++17 required ..."
#endif
#endif

#include <array>
#include <filesystem>
#include <cassert>
#include <memory>

using smart_buffer = std::shared_ptr<char>;

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

// decades old VERIFY macro
#define DBJ_VERIFY_(x, file, line ) if (!(x) ) dbj_terror( "  " #x " , failed at", file, line )
#define DBJ_VERIFY(x) DBJ_VERIFY_(x,__FILE__,__LINE__)

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

static const char * dbj_get_envvar(char * varname_)
{
	_ASSERTE(varname_);
	static std::array<char, 256>	bar{ {0} };
	bar.fill(0); // zero the buffer
	::SetLastError(0);
	if (1 > ::GetEnvironmentVariableA(varname_, bar.data(), (DWORD)bar.size()))
	{
		std::error_code  er_ = std::error_code(::GetLastError(), std::system_category());

		DBJ_VERIFY(!er_.message().c_str());
	}
	return bar.data();
}

/*
*/
static std::filesystem::path dbj_program_data_path()
{
	namespace fs = std::filesystem;

	std::string program_data{ dbj_get_envvar((char*)"ProgramData") };

	static fs::path prog_data_path_;
	prog_data_path_ = program_data;
	return prog_data_path_;
}


/*
https://stackoverflow.com/a/54491532/10870835
*/
template <typename CHR, typename win32_api>
static std::basic_string<CHR>
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
		result.resize(rl_ + rl_);
	}
}

static char * dbj_basename(
	char * full_path, 
	bool remove_suffix = true, 
	char delimiter = '\\' )
{
	char * base_ = strrchr(full_path, delimiter);
	base_ = (base_ == NULL ? full_path : base_);
	if (remove_suffix == false) return base_;
	char * dot_pos = strchr(base_, '.');
	if (dot_pos) *dot_pos = char(0);
	return base_;
}

std::string dbj_module_basename(HINSTANCE h_instance) {

	std::string module_path
		= string_from_win32_call<char>([h_instance](char* buffer, int size)
	{
		return GetModuleFileNameA(h_instance, buffer, size);
	});

	_ASSERTE(module_path.empty() == false);

	return { dbj_basename(module_path.data()) };
}

namespace dbj::ini
{
	/*
	   ini file descriptor

	   folder -- %programdata%\\dbj\\module_base_name
	   basename -- module_base_name + ".log"
	*/
	ini_file_descriptor ini_file() {

		auto base_name_ = dbj_module_basename(NULL);
		auto folder_
			= dbj_program_data_path()
			.concat("\\")
			.concat(dbj_programdata_subfolder);

		auto fp_ = folder_.concat(base_name_).concat(".ini");

		// NOTE: WIN32 STL filesystem is wchar_t "oriented"
		// so we have to jump through more hoops
		auto narrow = [](wchar_t const * wide_)
			-> std::string
		{
			std::wstring ws(wide_);
			return { ws.begin(), ws.end() };
		};

		std::string nf_ = narrow(folder_.c_str());
		std::string nfp_ = narrow(fp_.c_str());



		return ini_file_descriptor{
			smart_buffer(_strdup(nf_.data())),
			smart_buffer(_strdup(base_name_.data())),
			smart_buffer(_strdup(nfp_.data()))
		};
	}
} // dbj::ini
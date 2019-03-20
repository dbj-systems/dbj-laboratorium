
#include "dbj_util.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <string>
#include <memory>
#include <string_view>
#include <filesystem>

/*-----------------------------------------------------------------*/
extern std::string dbj_module_basename_2(HINSTANCE /*h_instance*/);
extern std::filesystem::path dbj_program_data_path();
/*-----------------------------------------------------------------*/

namespace dbj_local_log {

using namespace std;
using namespace std::string_view_literals;
namespace fs = std::filesystem;

/*
assure the presence of the folder: "%programdata%/DBJ_LOG_FILE_FOLDER"
*/
static auto 
  assure_log_file_folder ( fs::path const & the_last_part )  
	noexcept
	-> pair<fs::path, error_code>
{
	auto program_data_path = dbj_program_data_path() ;

	program_data_path += "\\";
	program_data_path += the_last_part;

	error_code e;
		auto dir = fs::directory_entry(program_data_path, e);
		// if we do not use e arg as above exception will be thrown
		// in case directory does not exist, instead we stay here
		// and create it
		if (e) {
			e.clear();
			// creates al the subdirs 
			if (! fs::create_directories(program_data_path, e))
			{
				return pair(program_data_path, e);
			}
		}
		// OK return
		e.clear();  dir.refresh(e); // must do refresh!
		_ASSERTE( dir.is_directory(e) );
		return pair(dir.path(), e);
}

	class log_file final {

		string	file_path_		{};
		int		old_std_err		{};
		FILE	*log_file_		{};

	public:

		log_file(const char * file_path_param) 
			: file_path_(file_path_param)
		{
		/*A true posix jamboree */
		old_std_err = _dup(2);   // "old_std_err" now refers to "stderr"

			if (old_std_err == -1)
			{
				perror("\n\n_dup( 2 ) failure -- " __FUNCSIG__ "\n\n");
				exit(1);
			}

			// here implement file creation with count in the file name added
			if (fopen_s(&log_file_, file_path_.data(), "w") != 0)
			{
				puts("\n\nCan't open file -- " __FUNCSIG__ "\n\n");
				puts(file_path_.data());
				exit(1);
			}

			// stderr now refers to file 
			if (-1 == _dup2(_fileno(log_file_), 2))
			{
				perror("\n\nCan't _dup2 stderr --"  __FUNCSIG__ "\n\n");
				exit(1);
			}
			// quick and dirty file header
			// if no errors, this file will stays empty
			// so header will have the role
			
			::fprintf(stderr, "DBJ++ log file | %s | %s\n", file_path_.data(),
				dbj_time_stamp(DBJ_TIME_STAMP_FULL_MASK));
		}

		~log_file() {
			// Flush stderr stream buffer 
			// not strictly necessary as
			// stderr is never buffered
			fflush(stderr);
			fclose(log_file_);
			// Restore original stderr
			_dup2(old_std_err, 2);
			_flushall();
			log_file_ = nullptr;
		}

		bool valid() const noexcept {
			return log_file_ != nullptr;
		}

		/*
		this exits if folder/file can not be made
		*/
		static log_file const & instance(
			const char * path_, 
			const char * name_ = nullptr
		)
		{
			auto initor = [&]() {
				auto[dir_path, e] = assure_log_file_folder(path_);

				if (e) {
					::wprintf(L"\nfailed to assure log folder %s\nerror is:\t%S", 
						dir_path.c_str(), 
						e.message().c_str()
					);
					exit(1);
				}
				// by default we use the module base name + ".log"
				fs::path full_path;
				if (name_ == nullptr) {
					std::string module_basename
						= dbj_module_basename_2(HINSTANCE(NULL));
					module_basename.append(".log");

					// concatenate dir path with file name
					full_path = dir_path.append(module_basename);
				}
				else
				{
					full_path = dir_path.append(name_);
				}
				// MSVC STL path uses wchar_t by default, ditto ...
				auto path_string{ full_path.string() };
				return log_file{ path_string.c_str() };
			};
				static log_file single_ = initor();
			return single_;
		}
	};

	static log_file const & log_file_instance 
		= log_file::instance(DBJ_LOG_FILE_FOLDER /*, DBJ_LOG_FILE_NAME*/);

	static int is_log_file_valid() {
		return log_file_instance.valid();
	}

} // dbj_local_log nspace

/////////////////////////////////////////////////////////////////////////

static CRITICAL_SECTION local_log;

void dbj_exit_write_to_local_log(void);

static BOOL init_critical() {
	InitializeCriticalSection(&local_log);
	atexit(dbj_exit_write_to_local_log);
	return TRUE;
}

static BOOL initialized = init_critical();

static void dbj_exit_write_to_local_log(void)
{
	if (!initialized)
		return;

	fflush(stderr);

	DeleteCriticalSection(&local_log);
	initialized = FALSE;
}

void dbj_local_log_file_write(const char * text_ ) 
{
	if (0 == initialized)
		return ;

	EnterCriticalSection(&local_log);
	_ASSERTE(text_);
	DBJ_VERIFY(dbj_local_log::is_log_file_valid());
	fprintf(stderr, "%s", text_);
	LeaveCriticalSection(&local_log);
}
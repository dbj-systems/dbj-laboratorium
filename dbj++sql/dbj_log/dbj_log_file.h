#pragma once
#include <io.h>
#include <string>
#include <string_view>
#include <filesystem>
#include "../dbj_util.h"

/*
tail of "%programdata%/dbj/dbj++sql"
*/

namespace dbj::db::log {

using namespace std;
using namespace std::string_view_literals;
namespace fs = std::filesystem;
namespace u = dbj::util;

constexpr inline auto LOG_FILE_FOLDER = "dbj\\dbj++sql"sv;
constexpr inline auto LOG_FILE_NAME = "dbj++sql.log"sv;

/*
assure the presence of the folder: "%programdata%/dbj/dbj++sql"
*/
inline auto 
  assure_log_file_folder ( fs::path const & the_last_part )  
	noexcept
	-> pair<fs::path, error_code>
{
	auto [ dbj_prog_data_path, e] = dbj::util::program_data_path() ;
	if (e) 	return pair(dbj_prog_data_path, e);

	dbj_prog_data_path += "\\"; 
	dbj_prog_data_path += the_last_part;

		e.clear();
		auto dir = fs::directory_entry(dbj_prog_data_path, e);
		// if we do not use e arg as above exception will be thrown
		// in case directory does not exist, instead we stay here
		// and create it
		if (e) {
			e.clear();
			// creates al the subdirs 
			if (! fs::create_directories(dbj_prog_data_path, e))
			{
				return pair(dbj_prog_data_path, e);
			}
		}
		// OK return
		e.clear();  dir.refresh(e); // must do refresh
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
			// if no errors, this file will stay empty
			// so header will have the role
			std::error_code ec;
			std::string tst = u::make_time_stamp(ec, u::TIME_STAMP_FULL_MASK); // size returned is max 23

			if (ec) {
				perror("\n\nCan't make the time stamp --" __FUNCSIG__ "\n\n" );
				perror(ec.message().c_str());
				exit(1);
			}
			
			::fprintf(stderr, "DBJ++SQL log file -- %s", file_path_.data());
			::fprintf(stderr, "\nCreated -- %s", tst.c_str());
			::fprintf(stderr, "\n\n");
		}

		~log_file() {
			// Flush stderr stream buffer 
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
		static log_file const & instance(const char * path_, const char * name_)
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
				// concatenate dir path with file name
				fs::path full_path = dir_path.append(name_);
				// MSVC STL path uses wchar_t by default, ditto ...
				auto path_string{ full_path.string() };
				return log_file{ path_string.c_str() };
			};
				static log_file single_ = initor();
			return single_;
		}
	};

	inline log_file const & log_file_instance 
		= log_file::instance(LOG_FILE_FOLDER.data(), LOG_FILE_NAME.data());

} // dbj::db::log nspace
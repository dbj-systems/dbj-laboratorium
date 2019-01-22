#pragma once
#include <io.h>
#include <string>
#include <string_view>
#include <filesystem>

/*
tail of "%programdata%/dbj/dbj++sql"
*/
#define LOG_FILE_FOLDER "dbj\\dbj++sql"
#define LOG_FILE_NAME "dbj++sql.log"

namespace dbj::db::log {

using namespace std;
using namespace std::string_view_literals;
using namespace std::filesystem;

[[nodiscard]] auto dbj_get_envvar(std::string_view varname_) noexcept
{
	std::array<char, 256>	prog_data{ {0} };
	std::error_code			ec_; // contains 0, that is OK val
	::SetLastError(0);
	if (1 > ::GetEnvironmentVariableA(varname_.data(), prog_data.data(), (DWORD)prog_data.size()))
	{
		ec_ = std::error_code(::GetLastError(), std::system_category());
	}
	return std::pair(std::string(prog_data.data()), ec_);
}

/*
assure the presence of the folder: "%programdata%/dbj/dbj++sql"
*/
inline auto 
  assure_log_file_folder ( path const & the_last_part )  
	noexcept
	-> pair<path, error_code>
{
	path dbj_prog_data_path;
	directory_entry dir ;

	auto[v, e] = dbj_get_envvar("ProgramData"); 
	if(e) 	return pair(the_last_part, e);

	dbj_prog_data_path = v;
	   
	dbj_prog_data_path += "\\"; 
	dbj_prog_data_path += the_last_part;

		e.clear();
		dir = directory_entry(dbj_prog_data_path, e);
		// if we do not use e arg as above exception will be thrown
		// in case directory does not exist, instead we stay here
		// and create it
		if (e) {
			e.clear();
			if (!create_directories(dbj_prog_data_path, e))
			{
				return pair(dbj_prog_data_path, e);
			}
		}
		// OK return
		e.clear();  dir.refresh(e);
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
			old_std_err = _dup(2);   // "old_std_err" now refers to "stderr"

			if (old_std_err == -1)
			{
				perror("\n\n_dup( 2 ) failure");
				exit(1);
			}

			if (fopen_s(&log_file_, file_path_.data(), "w") != 0)
			{
				puts("\n\nCan't open file :");
				puts(file_path_.data());
				exit(1);
			}

			// stderr now refers to file 
			if (-1 == _dup2(_fileno(log_file_), 2))
			{
				perror("\n\nCan't _dup2 stderr");
				exit(1);
			}
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
					wprintf(L"\nfailed to assure log folder %s\nerror is:\t%S", 
						dir_path.c_str(), 
						e.message().c_str()
					);
					exit(1);
				}
				// concatenate dir path with file name
				path log_file_path = name_;
				path full_path = dir_path.append(log_file_path.c_str());
				// MSVC STL path uses wchar_t by default, ditto ...
				string path_string = full_path.string();
				return log_file{ path_string.c_str() };
			};

			static log_file single_ = initor();

			return single_;
		}
	};

	inline log_file const & log_file_instance 
		= log_file::instance(LOG_FILE_FOLDER, LOG_FILE_NAME);

} // dbj::dbj::log nspace
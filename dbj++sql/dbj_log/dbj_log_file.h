#pragma once
#include <io.h>
#include <string>
#include <string_view>
/*
*/
#define LOG_FILE_PATH "c:/dbj/dbj++sql.log"

namespace dbj::db::log {

	using namespace std;
	using namespace std::string_view_literals;

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

		static log_file const & instance(const char * path_ )
		{
			static log_file single_(path_);
			return single_;
		}
	};

	inline log_file const & log_file_instance 
		= log_file::instance( LOG_FILE_PATH );

} // dbj::dbj::log nspace
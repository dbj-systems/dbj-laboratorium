#pragma once
#include "../win/dbj_win_inc.h"
#include "../win/dbj_win32.h"
#include <io.h>
//#include <string>
//#include <memory>
//#include <string_view>
#include <cstdio>
#include <future>
#include <filesystem>
#include <dbj++/core/dbj++core.h>

/*-----------------------------------------------------------------*/
extern "C" inline void dbj_on_exit_flush_stderr(void)
{
	fflush(stderr);
}
/*-----------------------------------------------------------------*/

namespace dbj {
	// under %programdata% on a local winddows machine
	constexpr inline const auto DBJ_LOCAL_FOLDER = "dbj";
}

namespace dbj::err {

	using namespace ::std;
	using smart_buffer = ::dbj::buf::yanb ;
	using namespace ::std::string_view_literals;
	namespace fs = ::std::filesystem;

	constexpr inline const unsigned DBJ_MAX_LOCAL_LOG_SIZE_KB = 0xFFFF;

	/*
	log file descriptor
	folder -- %programdata%\\dbj\\module_base_name
	basename -- module_base_name + ".log"
	*/
	struct log_file_descriptor final {
		smart_buffer folder;
		smart_buffer basename;
		smart_buffer fullpath;
	};

	log_file_descriptor log_file()
	{
		std::error_code ec_;
		dbj::buf::yanb programdata(dbj::core::util::program_data_path(ec_));

		_ASSERTE(!ec_);

		fs::path folder_ = programdata.data();
		folder_.concat("\\").concat(dbj::programdata_subfolder);

		dbj::buf::yanb base_name_ = ::dbj::win32::module_basename(NULL);

		fs::path full_path_(folder_);
		full_path_.concat(base_name_.data()).concat(".log");

		// NOTE: WIN32 STL filesystem is wchar_t "oriented"
		// so we have to jump through more hoops
		auto narrow = [](fs::path wide_)
			-> std::string
		{
			std::wstring ws(wide_);
			return { ws.begin(), ws.end() };
		};

		std::string nf_
			= narrow(folder_);
		std::string nfp_
			= narrow(full_path_);

		return log_file_descriptor{
			smart_buffer(nf_.data()),
			smart_buffer(base_name_.data()),
			smart_buffer(nfp_.data())
		};
	}



	namespace inner {

		// return true on truncation
		inline bool truncate_if_oversize(::dbj::buf::yanb	file_path_)
		{
			std::uintmax_t size_{};
			fs::path fp(file_path_.data());
			try {
				if (!exists(fp))
					return false;
				// The size of the file, in bytes.
				size_ = fs::file_size(fp);

				if (size_ > DBJ_MAX_LOCAL_LOG_SIZE_KB) {
					// truncate
					FILE *log_file_{};
					if (fopen_s(&log_file_, file_path_.data(), "w") != 0)
					{
						puts("\n\nCan't truncate the file -- " __FUNCSIG__ "\n\n");
						puts(file_path_.data());
						exit(1);
					}
				}
			}
			catch (fs::filesystem_error& e)
			{
				perror("\n\n");
				perror(e.what());
				perror("\n\n");
#ifdef _DEBUG
				::system("@echo.");
				::system("@pause");
				::system("@echo.");
#endif
				exit(1);
			}
			return true;
		}

		inline auto default_local_log_file_base_name() {
			std::string module_basename
				= ::dbj::win32::module_basename(HINSTANCE(NULL)).data();
			return module_basename.append(".log");
		}
		/*
		assure the presence of the folder: "%programdata%/DBJ_LOG_FILE_FOLDER"
		*/
		inline auto
			assure_log_file_folder(fs::path const & the_last_part)
			noexcept
			-> pair<fs::path, error_code>
		{
			error_code ec_;
			string program_data_path
				= ::dbj::core::util::program_data_path(ec_).data();

			_ASSERTE(!ec_);

			program_data_path += "\\";
			program_data_path += ::dbj::nano::transform_to<string, wstring>(
				the_last_part.c_str()
				);

			error_code e;
			auto dir = fs::directory_entry(program_data_path, e);
			// if we do not use e arg as above exception will be thrown
			// in case directory does not exist, instead we stay here
			// and create it
			if (e) {
				e.clear();
				// creates al the subdirs 
				if (!fs::create_directories(program_data_path, e))
				{
					return pair(program_data_path, e);
				}
			}
			// OK return
			e.clear();  dir.refresh(e); // must do refresh!
			_ASSERTE(dir.is_directory(e));
			return pair(dir.path(), e);
		}

		class log_file final {

			::dbj::buf::yanb	file_path_{};
			int		old_std_err{};
			FILE	*log_file_{};

		public:

			log_file(::dbj::buf::yanb file_path_param)
				: file_path_(file_path_param)
			{
				/*A true posix jamboree */
				old_std_err = _dup(2);   // "old_std_err" now refers to "stderr"

				if (old_std_err == -1)
				{
					perror("\n\n_dup( 2 ) failure -- " __FUNCSIG__ "\n\n");
					exit(1);
				}

				bool new_created = truncate_if_oversize(file_path_);

				if (fopen_s(&log_file_, file_path_.data(), "a") != 0)
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

				std::error_code ec_;
				// quick and dirty file header
				// if no errors, this file will stay empty
				// the header will have the role
				if (new_created) {
					::fprintf(stderr, "DBJ++ log file | %s | %s\n", file_path_.data(),
						::dbj::core::util::make_time_stamp(
							ec_,
							::dbj::core::util::TIME_STAMP_FULL_MASK)
						.data()
					);
				}
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
				const char * path_ = ::dbj::DBJ_LOCAL_FOLDER,
				const char * name_ = nullptr
			)
			{
				auto initor = [&]() {
					auto[dir_path, e] = assure_log_file_folder(path_);

					if (e) {
						perror("\nfailed to assure local log folder!\n");
						perror(__FILE__);
						exit(1);
					}
					::dbj::err::log_file_descriptor lfd_ 
						= ::dbj::err::log_file();

					return log_file{ lfd_.fullpath };
				};
				static log_file single_ = initor();
				return single_;
			}
		};

	} // inner

	inline inner::log_file const & log_file_instance()
	{
		static auto dumsy = [&]() {
			return atexit(dbj_on_exit_flush_stderr);
		}();
		return inner::log_file::instance( );
	}

	inline int is_log_file_valid() {
		return log_file_instance().valid();
	}

	/////////////////////////////////////////////////////////////////////////

	void local_log_file_write(const char * text_)
	{
		::dbj::sync::lock_unlock locker_;
		_ASSERTE(text_);
		DBJ_VERIFY(is_log_file_valid());
		std::fprintf(stderr, "%s", text_);
	}

	void local_log_file_write(const wchar_t * w_text_)
	{
		::dbj::sync::lock_unlock locker_;
		_ASSERTE(w_text_);
		DBJ_VERIFY(is_log_file_valid());
		std::fprintf(stderr, "%S", w_text_);
	}

	/*
	NOTE: no new lines or any other formating are
	added in here
	*/
	template<typename CHR>
	inline void async_log_write(
		std::basic_string_view<CHR> message
	) noexcept
	{
		_ASSERT(message.size() > 1);

		::dbj::sync::lock_unlock locker_;

		// pay attention, no new lines added here!
		auto log_to_stderr = [](std::basic_string_view<CHR> sv_)
		{
			_ASSERTE(sv_.size() > 0);
			local_log_file_write(sv_.data());
		};

		(void)std::async(std::launch::async, [&] {
			log_to_stderr(
				message.data()
			);
		});

		// temporary's dtor waits for log_to_stderr()
		// thus making this schema queued?
	}


} // dbj::err

#include "../dbj_gpl_license.h"



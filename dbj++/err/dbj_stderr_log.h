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
#include <dbj++/util/dbj++util.h>

/*-----------------------------------------------------------------*/
extern "C" inline void dbj_on_exit_flush_stderr(void)
{
	fflush(stderr);
}
/*-----------------------------------------------------------------*/

namespace dbj {
	// under %programdata% on a local windows machine
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
	struct log_file_descriptor final : 
		::dbj::util::file_descriptor  
	{
		virtual const char * suffix() const noexcept override { return ".log"; }
	};

	inline log_file_descriptor log_file() 
	{
		static log_file_descriptor lfd = [&]() 
		{
			static log_file_descriptor lfd_for_this_module_;
			::dbj::util::make_file_descriptor(lfd_for_this_module_);
			return lfd_for_this_module_;
		}();
		return lfd;
	}

	namespace inner {

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

				bool new_created = ::dbj::util::truncate_if_oversize
				(file_path_, DBJ_MAX_LOCAL_LOG_SIZE_KB );

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
					::fprintf(stderr, "\n\nDBJ++ log file | %s | %s\n", file_path_.data(),
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
					::dbj::err::log_file_descriptor lfd_ 
						= ::dbj::err::log_file();

					::dbj::util::assure_folder( lfd_ );

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



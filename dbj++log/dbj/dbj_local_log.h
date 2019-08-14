#pragma once
/*
include windows only from one place
and do it according to ancient windows lore
*/
#include <io.h>
//#include <string>
//#include <memory>
//#include <string_view>
#include <cstdio>
#include <future>
#include <filesystem>
#include <dbj++/core/dbj++core.h>
#include <dbj++/util/dbj++util.h>
#include <dbj++/win/dbj_win32.h>

/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/

namespace dbj::log {

	using namespace ::std;
	using smart_buffer = ::dbj::chr_buf::yanb;
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

			::dbj::chr_buf::yanb	file_path_{};
			int		old_std_err{};
			mutable FILE	*log_file_{};
			mutable bool opened = false;

		public:

			operator FILE * () const noexcept {
				return log_file_;
			}

			void open()  const noexcept {
				if (this->opened) return;
				dbj::errno_exit(
					fopen_s(&log_file_, file_path_.data(), "a"), __FILE__, __LINE__
				);
				this->opened = true;
			}
			void close() const noexcept {
				if (!this->opened) return;
				dbj::errno_exit(
					fclose (log_file_), __FILE__, __LINE__
				);
				this->opened = false;
			}

			log_file(::dbj::chr_buf::yanb file_path_param)
				: file_path_(file_path_param)
			{
				bool new_created
					= ::dbj::util::truncate_if_oversize
					(file_path_, DBJ_MAX_LOCAL_LOG_SIZE_KB);

#ifdef REASSIGN_STDERR_TO_FILE
				// Reassign "stderr" to file_path_
				errno_exit(
					freopen_s(&log_file_, file_path_.data(), "a", stderr), 
					__FILE__, __LINE__
				);

				// Flush stdout stream buffer so it goes to correct file
				errno_exit(fflush(stderr), __FILE__, __LINE__);
				clearerr_s(stderr);
#ifdef _DEBUG
				if (_isatty(_fileno(stderr))) {
					perror("stderr has not been redirected to a file\n");
					exit(1);
				}
				else {
					perror("stderr has been redirected to a file\n");
				}
#endif // _DEBUG
#else
				this->open();

				// Flush stdout stream buffer so it goes to correct file
				dbj::errno_exit(fflush(log_file_), __FILE__, __LINE__);
				clearerr_s(log_file_);
#endif // REASSIGN_STDERR_TO_FILE

				std::error_code ec_;
				// quick and dirty file header
				// if no errors, this file will stay empty
				// the header will have the role
				if (!new_created) return;
				// 1: do the SetLastError(0)
				(void)SetLastError(0);
				// 2: try the fprintf

				DBJ_FPRINTF(log_file_,"DBJ++ log file | %s | %s\n", file_path_.data(),
					::dbj::core::util::make_time_stamp(
						ec_,::dbj::core::util::TIME_STAMP_FULL_MASK).data()
					); 

				if (ec_) {
					DBJ_FPRINTF(log_file_, "std error in local log file constructor: %s", ec_.message().c_str() );
				}

				this->close();
			}

			void clear_after_myself_() noexcept {
				if (!log_file_)  return;
				try {
#ifdef REASSIGN_STDERR_TO_FILE
					// Flush stderr stream buffer 
					// not strictly necessary as
					// stderr is never buffered
					fflush(stderr);
					fclose(log_file_);
					// Restore original stderr
					_dup2(old_std_err, 2);
					_flushall();
					log_file_ = nullptr;
#else
					fflush(log_file_);
					this->close();
					_flushall();
					log_file_ = nullptr;
#endif
				}
				catch (...)
				{
					/* noop_ */
				}
			}

			~log_file() {
				clear_after_myself_();
			}

			bool valid() const noexcept {
				return log_file_ != nullptr;
			}

			/*
			this exits if folder/file can not be made
			*/
			static log_file const & instance(
				const char * path_ = ::dbj::dbj_programdata_subfolder,
				const char * name_ = nullptr
			)
			{
				auto initor = [&]() {
					::dbj::log::log_file_descriptor lfd_
						= ::dbj::log::log_file();

					::dbj::util::assure_folder(lfd_);

					return log_file{ lfd_.fullpath };
				};
				static log_file single_ = initor();
				return single_;
			}
		};

	} // inner

	inline inner::log_file const & log_file_instance()
	{
		return inner::log_file::instance();
	}

	inline int is_log_file_valid() {
		return log_file_instance().valid();
	}

	/////////////////////////////////////////////////////////////////////////
	namespace inner {
		/*
		be very carefull not to mix wide and narrow file writes!
		*/
		static bool & narrow_write () {
				static bool narrow_write_ = true ;
				return narrow_write_ ;
		}

		inline void local_log_file_write(const char * text_)
		{
			::dbj::sync::lock_unlock locker_;
			_ASSERTE(narrow_write());
			_ASSERTE(text_);
			_ASSERTE(is_log_file_valid());
			inner::log_file const & file_log_ = log_file_instance();
			file_log_.open();
			::fprintf(file_log_, "%s", text_);
			file_log_.close();
		}

		inline void local_log_file_write(const wchar_t * w_text_)
		{
			::dbj::sync::lock_unlock locker_;
			_ASSERTE( narrow_write() == false );
			inner::log_file const & file_log_ = log_file_instance();
			_ASSERTE(w_text_);
			_ASSERTE(is_log_file_valid());
			file_log_.open();
			::fwprintf(file_log_, L"%s", w_text_);
			file_log_.close();
		}
	} // inner

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
			inner::local_log_file_write(sv_.data());
		};

		(void)std::async(std::launch::async, [&] {
			log_to_stderr(
				message.data()
			);
		});

		// temporary's dtor waits for log_to_stderr()
		// thus making this schema queued?
	}

	template<typename CHR>
	inline void async_log_write(
		::dbj::chr_buf::yanb_t<CHR> message
	) noexcept
	{
		async_log_write(std::basic_string_view<CHR>{ message.data() });
	}

	template<typename CHR, size_t N>
	inline void async_log_write(
		const CHR(&message)[N]
	) noexcept
	{
		async_log_write(std::basic_string_view<CHR>{ message });
	}


} // dbj::log

#include "../dbj_gpl_license.h"



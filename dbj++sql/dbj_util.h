#pragma once
#include "stdafx.h"
/*
Just utilities
Collected here so we do not depend on libraries where they
might be already in existence.
*/
#ifndef DBJ_STR
#define DBJ_STR_(x) #x
#define DBJ_STR(x) DBJ_STR_(x)
#endif

#ifndef DBJ_EXP
#define DBJ_EXP_(x) (x)
#define DBJ_EXP(x) DBJ_EXP_(x)
#endif // !DBJ_EXP


#ifndef DBJ_VANISH
#define DBJ_VANISH(...) static_assert( (noexcept(__VA_ARGS__),true) );
#endif

namespace dbj {
	namespace util {

		// no can do, intelissense goes beserk  --> using namespace::std ;
		using namespace  ::std::literals::string_view_literals;
		namespace h =    ::std::chrono;
		namespace fs =   ::std::filesystem ;

		constexpr inline char const * TIME_STAMP_FULL_MASK
			= "%Y-%m-%d %H:%M:%S";

		constexpr inline char const * TIME_STAMP_SIMPLE_MASK
			= "%H:%M:%S";

		// time stamp size is max 22 + '\0'
		// updates ref to  std::error_code argument accordingly
		[[nodiscard]] inline std::string make_time_stamp(
			std::error_code & ec_,
			char const * timestamp_mask_ = TIME_STAMP_SIMPLE_MASK
		) noexcept
		{
			std::array<char, 32U> buf_arr{ {0} };
			char * buf = buf_arr.data();
			const size_t buf_len = buf_arr.size();

			ec_.clear();

			// Get the current time
			auto now = h::system_clock::now();
			// Format the date/time
			time_t now_tm_t = h::system_clock::to_time_t(now);
			struct tm  local_time_ {};

			errno_t posix_err_code = ::localtime_s(&local_time_, &now_tm_t);
			// leave the result empty if error
			if (posix_err_code > 0) {
				ec_ = std::make_error_code((std::errc)posix_err_code); return {};
			}

			strftime(buf, buf_len, timestamp_mask_, &local_time_);
			// Get the milliseconds
			int millis = h::time_point_cast<h::milliseconds>(now).time_since_epoch().count() % 100;
			// 
			const auto strlen_buf = std::strlen(buf);
			(void)::sprintf_s(buf + strlen_buf, buf_len - strlen_buf, ".%03d", millis);

			return { buf };
			// ec_ stays clear
		};

		/*
		returns {V, std::std::error_code},	caller must check
		*/
		[[nodiscard]] inline auto dbj_get_envvar(std::string_view varname_) noexcept
		{
			_ASSERTE(!varname_.empty());
			std::array<char, 256>	bar{ {0} };
			std::error_code			ec_; // contains 0 as the OK val
			::SetLastError(0);
			if (1 > ::GetEnvironmentVariableA(varname_.data(), bar.data(), (DWORD)bar.size()))
			{
				ec_ = std::error_code(::GetLastError(), std::system_category());
			}
			return std::pair(std::string(bar.data()), ec_);
		}

		/*
		returns {V, std::std::error_code},	caller must check
		*/
		[[nodiscard]] inline auto program_data_path() noexcept {
			fs::path prog_data_path_;
			auto ve_pair = dbj_get_envvar("ProgramData");
			// if error return
			// thus prog_data_path_ is empty
			if (ve_pair.second) return std::pair(prog_data_path_, ve_pair.second);
			// ok return
			// prog_data_path_ get the path string
			prog_data_path_ = ve_pair.first;
			return std::pair(prog_data_path_, ve_pair.second);
		}

		namespace fmt {
			/*
			somewhat inspired with
			https://msdn.microsoft.com/en-us/magazine/dn913181.aspx
			*/
			template <typename T>
			T & frm_arg(const T & value) noexcept
			{
				return value;
			}
			// no pass by value allowed
			template <typename T> T & frm_arg(T && value) = delete;

			template <typename T>
			T const * frm_arg(std::basic_string<T> const & value) noexcept
			{
				return value.c_str();
			}

			template <typename T>
			T const * frm_arg(std::basic_string_view<T> const & value) noexcept
			{
				return value.data();
			}
			/*
			inspired by
			https://stackoverflow.com/a/39972671/10870835
			*/
			template<typename ... Args>
			inline std::string
				dbj_format( std::string_view format_, Args const & ... args)
				noexcept
			{
				static_assert(sizeof...(args) < 255, "\n\nmax 255 arguments allowed\n");
				const auto fmt = format_.data();
				// 1: what is he size required
				const size_t size = std::snprintf(nullptr, 0, fmt, frm_arg(args) ...) + 1;
				// 2: use it at runtime
				auto buf = std::make_unique<char[]>(size);
				// each arg becomes arg to the frm_arg() overload found
				std::snprintf(buf.get(), size, fmt, frm_arg(args) ...);
				auto res = std::string(buf.get(), buf.get() + size - 1);
				return res;
			}

		} // fmt

		template<typename ... Args>
		inline void
			dbj_print( std::string_view format_, Args const & ... args)
			noexcept
		{
			std::wprintf(L"%S", fmt::dbj_format(format_, args...).c_str() );
		}

	} // util
} // dbj




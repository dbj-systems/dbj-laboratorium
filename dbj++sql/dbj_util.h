#pragma once
/*
Just utilities
*/
#include <string>
#include <string_view>
#include <filesystem>
#include <chrono>
#include <array>

namespace dbj::util {
	
	using namespace::std::string_view_literals;
	namespace s =  ::std;
	namespace fs = ::std::filesystem;
	namespace h =  ::std::chrono ;

	constexpr inline char const * TIME_STAMP_FULL_MASK
		= "%Y-%m-%d %H:%M:%S";

	constexpr inline char const * TIME_STAMP_SIMPLE_MASK
		= "%H:%M:%S";

	// time stamp size is max 22 + '\0'
	// updates ref to  error_code argument accordingly
	[[nodiscard]] inline s::string make_time_stamp(
		s::error_code & ec_ , 
		char const * timestamp_mask_ = TIME_STAMP_SIMPLE_MASK
	) noexcept
	{
		s::array<char, 32U> buf_arr{ {0} };
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
			ec_ = std::make_error_code((s::errc)posix_err_code); return {};
		}

		s::strftime(buf, buf_len, timestamp_mask_, &local_time_);
		// Get the milliseconds
		int millis = h::time_point_cast<h::milliseconds>(now).time_since_epoch().count() % 100;
		// 
		const auto strlen_buf = ::strlen(buf);
		(void)::sprintf_s(buf + strlen_buf, buf_len - strlen_buf, ".%03d", millis);

		return { buf };
		// ec_ stays clear
	};

	[[nodiscard]] inline auto dbj_get_envvar(s::string_view varname_) noexcept
	{
		_ASSERTE(!varname_.empty());
		s::array<char, 256>	bar{ {0} };
		s::error_code			ec_; // contains 0 as the OK val
		::SetLastError(0);
		if (1 > ::GetEnvironmentVariableA(varname_.data(), bar.data(), (DWORD)bar.size()))
		{
			ec_ = s::error_code(::GetLastError(), s::system_category());
		}
		return s::pair(s::string(bar.data()), ec_);
	}

	[[nodiscard]] inline auto program_data_path() noexcept {
		fs::path prog_data_path_;
		auto[v, e] = dbj_get_envvar("ProgramData");
		// if error return
		// thus prog_data_path_ is empty
		if (e) 	return s::pair(prog_data_path_, e);
		// ok return
		// prog_data_path_ get the path string
		prog_data_path_ = v;
		return s::pair(prog_data_path_, e);
	}

	namespace str {
	/*
	somewhat inspired with
	https://msdn.microsoft.com/en-us/magazine/dn913181.aspx
	*/
		template <typename T>
		T frm_arg(T value) noexcept
		{
			return value;
		}

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
https://stackoverflow.com/a/39972671/10870835
*/
template<typename ... Args>
inline std::string 
	dbj_format(const std::string& format_, Args const & ... args)
noexcept
{
	const auto fmt = format_.c_str();
	const size_t size = std::snprintf(nullptr, 0, fmt, frm_arg(args) ...) + 1;
	auto buf = std::make_unique<char[]>(size);
	std::snprintf(buf.get(), size, fmt, frm_arg(args) ...);
	auto res = std::string(buf.get(), buf.get() + size - 1);
	return res;
}

	} // str
} // dbj::util

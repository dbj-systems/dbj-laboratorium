#ifndef _DBJ_UTILS_CORE_INC
#define _DBJ_UTILS_CORE_INC

// DBJ! This is circular include ... Bollocks ..
#include "dbj_runtime.h"

namespace dbj {
	// the first time dbj::core namespace was introduced was here
	namespace core {
		namespace util {

			// no can do, intelisense goes berserk  --> using namespace::std ;
			using namespace  ::std::literals::string_view_literals;
			namespace h = ::std::chrono;

			using buf_helper = typename ::dbj::vector_buffer<char>;
			using buf_type = typename buf_helper::narrow;

			// this is *very* tricky to get right
			template<typename T>
			constexpr T midpoint(T a, T b) noexcept
			{
				// only numbers please
				static_assert(std::is_arithmetic_v<T>);

				return T((a)+((b)-(a)) / 2);
			}

			// as per http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0811r2.html
			constexpr int midpoint(int a, int b) noexcept {
				using U = make_unsigned_t<int>;
				return int(U(a) + (U(b) - U(a)) / 2);
			}

			constexpr inline char const* TIME_STAMP_FULL_MASK
				= "%Y-%m-%d %H:%M:%S";

			constexpr inline char const* TIME_STAMP_SIMPLE_MASK
				= "%H:%M:%S";

			// time stamp size is max 22 + '\0'
			// updates the ref to std::error_code argument accordingly
			[[nodiscard]]
			inline buf_type
				make_time_stamp(
					std::error_code& ec_,
					char const* timestamp_mask_ = TIME_STAMP_SIMPLE_MASK
				) noexcept
			{
				ec_.clear();
				constexpr size_t buf_len = 64U;
				std::array<char, buf_len>  buffer_{ {0} };
				char* buf = buffer_.data();
				// Get the current time
				auto now = h::system_clock::now();
				// Format the date/time
				time_t now_tm_t = h::system_clock::to_time_t(now);
				struct tm  local_time_ {};

				errno_t posix_err_code = ::localtime_s(&local_time_, &now_tm_t);
				// return empty buffer if error
				if (posix_err_code > 0) {
					ec_ = std::make_error_code((std::errc)posix_err_code); return {};
				}

				strftime(buf, buf_len, timestamp_mask_, &local_time_);
				// Get the milliseconds
				int millis = h::time_point_cast<h::milliseconds>(now).time_since_epoch().count() % 100;
				// 
				const auto strlen_buf = std::strlen(buf);
				(void)::sprintf_s(buf + strlen_buf, buf_len - strlen_buf, ".%03d", millis);

				return buf_helper::make(buffer_.data());

				// ec_ stays clear
			};

			/*	caller must check std::error_code ref arg	*/
			[[nodiscard]] inline
				buf_type
				dbj_get_envvar(std::string_view varname_, std::error_code& ec_) noexcept
			{
				_ASSERTE(!varname_.empty());
				constexpr size_t buflen_ = 256U;
				std::array<char, buflen_ > bar;
				ec_.clear(); // the OK 
				::SetLastError(0);
				if (1 > ::GetEnvironmentVariableA(varname_.data(), bar.data(), (DWORD)buflen_))
				{
					ec_ = std::error_code(::GetLastError(), std::system_category());
				}
				return buf_helper::make(bar.data());
			}

			/*	caller must check the ec_	*/
			[[nodiscard]] inline auto
				program_data_path(std::error_code& ec_) noexcept -> buf_type
			{
				return dbj_get_envvar("ProgramData", ec_);
			}

		} // util

		/*
		unvarnished is synonim for literal
		it seems MSVC has a problem if nemapsace is called 'literals'
		*/
		namespace unvarnished {
			/*
			numeric literals user defined literal. usege:

				using namespace dbj::core::literals ;

				// std::array{ '1','2','3' }
				constexpr auto a_1 = 123_std_char_array ;
				// std::array{ '0','x','1','2' }
				constexpr auto a_2 = 0x12_std_char_array ;
				// std::array{ '4'.'2','.','1','3' }
				constexpr auto a_3 = 42.13_std_char_array ;
			*/
			template< char ... Chs >
			inline constexpr decltype(auto) operator"" _std_char_array()
			{
				return  std::array{ Chs... , char(0) };
			}

		} // unvarnished
	}// core
} // dbj


#endif // !_DBJ_UTILS_CORE_INC

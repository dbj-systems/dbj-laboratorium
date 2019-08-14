#pragma once

#ifndef _HAS_CXX17
	#error C++17 please ...
#endif

namespace dbj {
	// the first time dbj::core namespace was introduced was here
	namespace core {
		namespace util {

			// no can do, intelisense goes berserk  --> using namespace::std ;
			using namespace  ::std::literals::string_view_literals;
			namespace h = ::std::chrono;

			using buf_type = typename ::dbj::chr_buf::narrow_type ;

			// this is *very* tricky to get right
			template<typename T>
			constexpr T midpoint(T a, T b) noexcept 
			{
				// only numbers please
				static_assert( std::is_arithmetic_v<T> );

				return T((a) + ((b) - (a)) / 2);
			}

			// as per http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0811r2.html
			constexpr int midpoint(int a, int b) noexcept {
				using U = make_unsigned_t<int>;
				return int(U(a) + (U(b) - U(a)) / 2);
			}

			constexpr inline char const * TIME_STAMP_FULL_MASK
				= "%Y-%m-%d %H:%M:%S";

			constexpr inline char const * TIME_STAMP_SIMPLE_MASK
				= "%H:%M:%S";

			// time stamp size is max 22 + '\0'
			// updates the ref to std::error_code argument accordingly
			[[nodiscard]]
			inline buf_type
				make_time_stamp(
					std::error_code & ec_,
					char const * timestamp_mask_ = TIME_STAMP_SIMPLE_MASK
				) noexcept
			{
				ec_.clear();
				constexpr size_t buf_len = 64U;
				std::array<char, buf_len>  buffer_{ {0} };
				char * buf = buffer_.data();
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

				buf_type smart_ptr_buffer;

				dbj::chr_buf::core::assign(smart_ptr_buffer , buffer_.data() );

				return smart_ptr_buffer;
				// ec_ stays clear
			};

			/*	caller must check std::error_code ref arg	*/
			[[nodiscard]] inline 
				buf_type
				dbj_get_envvar(std::string_view varname_, std::error_code & ec_) noexcept
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

				buf_type smart_ptr_buffer;
					dbj::chr_buf::core::assign(smart_ptr_buffer, bar.data());

				return smart_ptr_buffer;
			}

			/*	caller must check the ec_	*/
			[[nodiscard]] inline auto
				program_data_path( std::error_code & ec_ ) noexcept -> buf_type
			{
					return dbj_get_envvar("ProgramData", ec_ );
			}
		} // util
	}// core
} // dbj
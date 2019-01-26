#pragma once
/*
Just utilities
Coollected here so we do not depend on libraries where they
might be alreay in existence.
*/
#include <string>
#include <string_view>
#include <filesystem>
#include <chrono>
#include <array>

namespace dbj {
	namespace util {

		using namespace::std ;
		using namespace::std::string_view_literals;
		namespace h = ::std::chrono;
		namespace fs = ::std::filesystem ;

		constexpr inline char const * TIME_STAMP_FULL_MASK
			= "%Y-%m-%d %H:%M:%S";

		constexpr inline char const * TIME_STAMP_SIMPLE_MASK
			= "%H:%M:%S";

		// time stamp size is max 22 + '\0'
		// updates ref to  error_code argument accordingly
		[[nodiscard]] inline string make_time_stamp(
			error_code & ec_,
			char const * timestamp_mask_ = TIME_STAMP_SIMPLE_MASK
		) noexcept
		{
			array<char, 32U> buf_arr{ {0} };
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
				ec_ = std::make_error_code((errc)posix_err_code); return {};
			}

			strftime(buf, buf_len, timestamp_mask_, &local_time_);
			// Get the milliseconds
			int millis = h::time_point_cast<h::milliseconds>(now).time_since_epoch().count() % 100;
			// 
			const auto strlen_buf = ::strlen(buf);
			(void)::sprintf_s(buf + strlen_buf, buf_len - strlen_buf, ".%03d", millis);

			return { buf };
			// ec_ stays clear
		};

		/*
		returns {V, std::error_code},	caller must check
		*/
		[[nodiscard]] inline auto dbj_get_envvar(string_view varname_) noexcept
		{
			_ASSERTE(!varname_.empty());
			array<char, 256>	bar{ {0} };
			error_code			ec_; // contains 0 as the OK val
			::SetLastError(0);
			if (1 > ::GetEnvironmentVariableA(varname_.data(), bar.data(), (DWORD)bar.size()))
			{
				ec_ = error_code(::GetLastError(), system_category());
			}
			return pair(string(bar.data()), ec_);
		}

		/*
		returns {V, std::error_code},	caller must check
		*/
		[[nodiscard]] inline auto program_data_path() noexcept {
			fs::path prog_data_path_;
			auto ve_pair = dbj_get_envvar("ProgramData");
			// if error return
			// thus prog_data_path_ is empty
			if (ve_pair.second) return pair(prog_data_path_, ve_pair.second);
			// ok return
			// prog_data_path_ get the path string
			prog_data_path_ = ve_pair.first;
			return pair(prog_data_path_, ve_pair.second);
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

		namespace init {

			//template<typename T, typename ... Args>
			//using initor_function = T const & (*)(Args ...);

			template<typename T>
			struct initor_function final
			{
				using value_type = T;

				// T must be movable 
				/*
				template<typename ...Args>
				T  operator () ( Args...args) const
				{
					return T(args...);
				}
				*/

				// T does not need to be movable 
				// T &, mus tbe first argument
				template<typename ...Args>
				T & operator () (T & result_, Args...args) const
				{
					return result_ = T(args...);
				}
			};

			template<typename T, typename ... Args>
			inline T const &
				universal_initor_(Args ... args) {
				/* this is where T singleton is made */
				static T single_instance(args ...);
				return single_instance;
			};

			/*
			dbj's "make it once" idiom
			*/
			template<typename T, typename F>
			struct once final {

				using value_type = T;

				static_assert(
					std::is_constructible_v <T>,
					"\nmake_once<T>() -- T must be constructible\n"
					);

				template<typename ... Args>
				static T const & make(Args ... args)
				{
					/* this is resilient in presence of multiple threads */
					static T const & singleton_ = F(args...);
					/* return by ref. so type can be non-movable and non-copyable if required */
					return singleton_;
				}
			}; // once maker
		} // dbj::init
	} // util
} // dbj

#define DBJ_UTIL_TEST
#ifdef DBJ_UTIL_TEST

namespace whatever {
	using namespace std;
	string highlander() { return "Highlander"; };
	//-------------------------------------------------------

	template <auto RT> struct starter;

	template<typename RT, typename... ATs, RT(*pF)(ATs...)>
	struct starter<pF>  final
	{
		static RT const & call(ATs ... args) {
			static RT const & singleton_ = pF(args...);
			return singleton_;
		}
	};
	//-------------------------------------------------------

	template<typename RT, typename... ATs>
	inline RT const & once(RT(*pF)(ATs...), ATs ... args)
	{
		static RT const & singleton_ = pF(args...);
		return singleton_;
	}

	//----------------------------------------------------------------

	using namespace dbj::util::init;
	using my_initor = initor_function<int>;

	template<typename T>
	auto maker()
	{
		return [](T & t_, auto ... args) -> T & {
			static initor_function<T> inf_type;
			return inf_type(t_, args...);
		};
	}

	int getint(int arg) { return arg; }

	bool test_the_initor()
	{
		using ::dbj::util::str::dbj_format;

		using  make_highlander = starter<highlander>;
//		auto the_only_one = make_highlander::call();

//		::wprintf("%S", dbj_format("%s", the_only_one));

		auto forever42a = once(getint, 42);

		return true;
	}

	inline auto run_test = test_the_initor();
}

#endif // DBJ_UTIL_TEST


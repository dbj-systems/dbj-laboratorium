#pragma once
#include <future>
#include <string_view>
#include <ctime>
#include <chrono>
/*
async queued nano log olympics, are opened

todo: redirecting stderr, timestamps, colors and all that jazz
*/
namespace dbj::db {
	
	using namespace std;
	using namespace std::chrono;
	using namespace std::string_view_literals ;

	namespace {

		// time stamp made size is 22 + '\0'
		// returns POSIX retval or 0 on no error
		[[nodiscard]] inline int make_time_stamp ( string & result_ ) noexcept
		{
			result_.clear();
			// almost C
			array<char, 32U> buf_arr{ {0} };
			char * buf = buf_arr.data();
			const size_t buf_len = buf_arr.size();

			// Get the current time
			auto now = system_clock::now();
			// Format the date/time
			time_t now_tm_t = system_clock::to_time_t(now);
			struct tm  local_time_ {};

			errno_t posix_err_code = ::localtime_s(&local_time_, &now_tm_t);
			// leave the result empty if error
			if (posix_err_code > 0) return posix_err_code;

			std::strftime(buf, buf_len, "%Y-%m-%d %H:%M:%S", &local_time_);
			// Get the milliseconds
			int millis = time_point_cast<milliseconds>(now).time_since_epoch().count() % 100;
			// Note use snprintf for gcc
			const auto strlen_buf = ::strlen(buf);
			(void)::sprintf_s(buf + strlen_buf, buf_len - strlen_buf, ".%03d", millis);
			result_ = buf;
			return 0; // no posix error
		};

		// TBDone
		constexpr size_t prompt_msg_len = 24U;
		constexpr size_t prompt_tst_len = 24U;

		[[nodiscard]] inline auto make_prompt( string_view msg_ ) noexcept -> std::string
		{
			string tst(23, '\0');
			auto posix_errno [[maybe_unused]] = make_time_stamp(tst); // size returned is 23
#ifdef _DEBUG
			const char * one_[[maybe_unused]] = tst.c_str();
#endif
			const char * two_ = 
				msg_.size() > prompt_msg_len ? msg_.substr(0, prompt_msg_len).data() : msg_.data() ;

			array<char, 64> buf{ {0} };
			auto retval [[maybe_unused]] = ::sprintf_s(buf.data(), buf.size(),
				"\n|%24s|%24s| ", tst.c_str(), two_
			);
			return buf.data();
		}
	}

	namespace log {
		using namespace std::literals::string_view_literals;

		namespace {
			/*
			prompt and message1 are mandatory
			message2 is optional

			NOTE: no new lines or any other formating are 
			added in here
			*/
			inline void log_imp(
				std::string_view prompt_,
				std::string_view message1
				, std::string_view message2 = ""sv
			) noexcept
			{
				_ASSERT(prompt_.size() > 1);
				_ASSERT(message1.size() > 1);
				// pay attention, no new lines here or any other formating
				auto log_to_stderr = []( std::string_view data_) { ::fprintf(stderr, "%s", data_.data()); };
				(void)std::async(std::launch::async, [&] { log_to_stderr(prompt_); });
				(void)std::async(std::launch::async, [&] { log_to_stderr(message1); });

				if (message2.size() >1 )
				(void)std::async(std::launch::async, [&] { log_to_stderr(message2); });

				// temporary's dtor waits for log_to_stderr()
				// thus making this schema queued
			}
		}

		void info(std::string_view msg1_, std::string_view msg2_= ""sv) noexcept
		{
			static constexpr auto prompt_ = "info"sv;
			log_imp(make_prompt(prompt_), msg1_, msg2_ );
		}

		void warning(std::string_view msg1_, std::string_view msg2_= ""sv) noexcept
		{
			static constexpr auto prompt_ = "warning"sv;
			log_imp(make_prompt(prompt_), msg1_, msg2_ );
		}

		void error(std::string_view msg1_, std::string_view msg2_ = ""sv) noexcept
		{
			static constexpr auto prompt_ = "error"sv;
			log_imp(make_prompt(prompt_), msg1_, msg2_ );
		}

	}
}
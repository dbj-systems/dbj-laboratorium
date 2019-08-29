#pragma once
#include <future>
#include <string_view>
#include <ctime>
#include <chrono>
#include "../dbj_util.h"
/*
async queued nano log olympics, are opened

todo: redirecting stderr, timestamps, colors and all that jazz
*/
namespace dbj::db {
	
	using namespace std;
	using namespace std::chrono;
	using namespace std::string_view_literals ;

	namespace u = dbj::util;

	namespace {

		namespace s = ::std;

		constexpr inline size_t prompt_msg_len = 12U;
		constexpr inline size_t prompt_tst_len = 24U;

		[[nodiscard]] inline s::string 
			make_prompt( s::string_view msg_, s::error_code & ec_ ) 
		noexcept 
		{
			ec_.clear() ;
			auto tst = u::make_time_stamp(ec_); // size returned is max 23
			if (ec_) return {};
#ifdef _DEBUG
			const char * one_[[maybe_unused]] = tst.c_str();
#endif
			const char * two_ = 
				msg_.size() > prompt_msg_len ? msg_.substr(0, prompt_msg_len).data() : msg_.data() ;

			s::array<char, 64> buf{ {0} };
			auto retval [[maybe_unused]] = ::sprintf_s(buf.data(), buf.size(),
				"\n|%s|%8s| ", tst.c_str(), two_
			);
			return { buf.data() } ;
			// ec_ stays clear
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
				, std::string_view message2 = "  "sv
			) noexcept
			{
				_ASSERT(prompt_.size() > 1);
				_ASSERT(message1.size() > 1);
				// pay attention, no new lines here or any other formating
				auto log_to_stderr = []( char const * s1, char const * s2, char const * s3 ) {
					::fprintf(stderr, "%s %s %s", s1,s2,s3);
				};

				(void)std::async(std::launch::async, [&] { 
					log_to_stderr(
						prompt_.data(), message1.data(), message2.data()
					);
				});

				// temporary's dtor waits for log_to_stderr()
				// thus making this schema queued
			}
		}

		inline void info(std::string_view msg1_, std::string_view msg2_= ""sv) noexcept
		{
			static constexpr auto prompt_ = "info"sv;
			error_code ec;
			log_imp(make_prompt(prompt_, ec ), msg1_, msg2_ );
			_ASSERTE(!ec);
		}

		inline void warning(std::string_view msg1_, std::string_view msg2_= ""sv) noexcept
		{
			static constexpr auto prompt_ = "warning"sv;
			error_code ec;
			log_imp(make_prompt(prompt_, ec), msg1_, msg2_ );
			_ASSERTE(!ec);
		}

		inline void error(std::string_view msg1_, std::string_view msg2_ = ""sv) noexcept
		{
			static constexpr auto prompt_ = "error"sv;
			error_code ec;
			log_imp(make_prompt(prompt_, ec), msg1_, msg2_ );
			_ASSERTE(!ec);
		}
	}
}
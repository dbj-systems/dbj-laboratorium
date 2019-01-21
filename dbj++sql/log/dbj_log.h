#pragma once
#include <future>
#include <string_view>
/*
async queued nano log olympics, are opened

todo: redirecting stderr, timestamps, colors and all that jazz
*/
namespace dbj::db {

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

		void info(std::string_view sv_, std::string_view log_message= ""sv) noexcept
		{
			static constexpr auto prompt = "\ninfo : "sv;
			log_imp(prompt, sv_, log_message );
		}

		void warning(std::string_view sv_, std::string_view log_message= ""sv) noexcept
		{
			static constexpr auto prompt = "\nwarning : "sv;
			log_imp(prompt, sv_, log_message );
		}

		void error(std::string_view sv_, std::string_view log_message = ""sv) noexcept
		{
			static constexpr auto prompt = "\nerror : "sv;
			log_imp(prompt, sv_, log_message );
		}

	}
}
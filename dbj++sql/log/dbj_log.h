#pragma once
#include <future>
#include <string_view>
/*
async queued nano log olympic are opened

todo: redirecting stderr, timestamps, colors and all that jazz
*/
namespace dbj::db {

	namespace log {
		using namespace std::literals::string_view_literals;

		namespace {
			void log_imp(
				std::string_view prompt_,
				std::string_view sv_
			) noexcept
			{
				// pay attention, no new lines here
				auto log_to_stderr = []( std::string_view data_) { ::fprintf(stderr, "%s", data_.data()); };
				(void)std::async(std::launch::async, [&] { log_to_stderr(prompt_); });
				(void)std::async(std::launch::async, [&] { log_to_stderr(sv_); });
				// temporary's dtor waits for log_to_stderr()
				// thus making this schema queued
			}
		}

		void info(std::string_view sv_) noexcept
		{
			static constexpr auto prompt = "\ninfo : "sv;
			log_imp(prompt, sv_);
		}
		void error(std::string_view sv_) noexcept
		{
			static constexpr auto prompt = "\nerror : "sv;
			log_imp(prompt, sv_);
		}

	}
}
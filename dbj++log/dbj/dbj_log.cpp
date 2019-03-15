#pragma once
#include <future>
#include <cassert>
#include <string_view>
#include <ctime>
#include <chrono>
#include <array>
#include "dbj_util.h"
/*
The roadmap:
async queued nano log olympics, are opened
todo: redirecting stderr, timestamps, colors and all that jazz
*/
using namespace std;
using namespace std::chrono;
using namespace std::literals::string_view_literals;

/*
NOTE: no new lines or any other formating are
added in here
*/
extern "C" static void async_log_write(
	std::string_view message
) noexcept
{
	_ASSERT(message.size() > 1);
	// pay attention, no new lines added here!
	auto log_to_stderr = [](char const * s1) {
		::fprintf(stderr, "\n%s", s1);
	};

	(void)std::async(std::launch::async, [&] {
		log_to_stderr(
			message.data()
		);
	});

	// temporary's dtor waits for log_to_stderr()
	// thus making this schema queued, NOT
}

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

CRITICAL_SECTION local_log;

void dbj_exit_write_to_local_log(void);

static BOOL init_critical() {
	InitializeCriticalSection(&local_log);
	atexit(dbj_exit_write_to_local_log);
	return TRUE;
}

static BOOL initialized = init_critical();

void dbj_exit_write_to_local_log(void)
{
	if (!initialized)
		return;
	DeleteCriticalSection(&local_log);
	initialized = FALSE;
}

void dbj_write_to_local_log( char * const msg_ ,  int locked_ ) 
{
	DBJ_VERIFY(msg_);
	DBJ_VERIFY(initialized);
	   
	if (! locked_ ) ::EnterCriticalSection(&local_log);
	async_log_write(msg_);
	if (!locked_) ::LeaveCriticalSection(&local_log);
}





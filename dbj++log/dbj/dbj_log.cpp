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
	auto log_to_stderr = [](char const * s1) 
	{
		_ASSERTE(s1);
		dbj_local_log_file_write(s1);
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

static void dbj_exit_write_to_local_log(void)
{
	if (!initialized)
		return;
	DeleteCriticalSection(&local_log);
	initialized = FALSE;
}

void dbj_write_to_local_log(
	char * priority_name_ ,
	char * timestamp_rfc3164,
	char * local_hostname,
	char * syslog_ident,
	char * syslog_procid_str,
	int lock_for_mt ,
	const char * the_message )
{
	DBJ_VERIFY(priority_name_ && timestamp_rfc3164 && local_hostname 
		&& syslog_ident && syslog_procid_str && the_message);

	DBJ_VERIFY(initialized);

	char datagramm[BUFSIZ * 2] = { 0 }; // yes we hardcode the datagramm size to 1KB
	static const unsigned datagramm_size = sizeof(datagramm);
	   
	if ( lock_for_mt) ::EnterCriticalSection(&local_log);

	DBJ_VERIFY( 1 < sprintf_s(datagramm, datagramm_size,
		"\n%8s |%20s |%16s |%16s%s | %s",
		priority_name_, timestamp_rfc3164 ,
		local_hostname, syslog_ident, syslog_procid_str, the_message));

	async_log_write(datagramm);
	// above requires write buffer and mechanism that will empty the buffer on exit?
	// No! stderr is never buffered ... each write immediately goes to disk.

	if ( lock_for_mt) ::LeaveCriticalSection(&local_log);
}





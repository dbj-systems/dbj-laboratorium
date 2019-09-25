/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
#include "dbj_file_handle.h"

namespace dbj::simplelog {
	extern "C" {

#endif // __cplusplus

#define LOG_VERSION "0.5.0"
#define LOG_USE_COLOR

		bool enable_vt_mode();

		// bear in mind in C++ this type full name is
		// dbj::simplelog::log_lock_function_ptr 
		typedef void (*log_lock_function_ptr)(void* udata, int lock);

		enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

		void log_set_udata(void* udata);
		void log_set_lock(log_lock_function_ptr fn);
		void log_set_fp(FILE* fp);
		void log_set_level(int level);
		void log_set_quiet(int enable);
		/* 0 do not add file line to log lines stamp, 1 add */
		void log_set_fileline( unsigned );

		void log_log(int level, const char* file, int line, const char* fmt, ...);

#ifdef __cplusplus
	} // extern "C" 
} // namespace dbj::simplelog 
#endif // __cplusplus

#ifdef __cplusplus
#include <mutex>

namespace dbj::simplelog {
	namespace mt {
		/*
		C++ MT usage
	   */
		struct log_protector final
		{
			//  process wide mutex instance
			inline static std::mutex log_protector_mutex{};
			// one per instance
			// try to guard the proc wide mutex as soon as constructed
			log_protector() {
				log_protector_mutex.lock();
			}

			~log_protector() {
				// once per instance
				// release the guard on the proc wide mutex instance
				log_protector_mutex.unlock();
			}
		};

		// scafolding for MT resilience 
		// the logic here is imposed by simple protector implementation
		inline log_protector* global_log_protector_ptr = nullptr;

		// typedef void (*log_lock_function_ptr)(void* udata, int lock);
		extern "C" inline void  protector_function(void* udata, int lock)
		{
			log_protector* log_protector_ptr = (log_protector*)udata;

			if (lock)
			{
				_ASSERTE(global_log_protector_ptr == nullptr);
				global_log_protector_ptr = new log_protector{};
			}
			else {
				// unlock
				_ASSERTE(global_log_protector_ptr != nullptr);
				delete global_log_protector_ptr;
				global_log_protector_ptr = nullptr;
			}
		}
	} // mt

		/*
	in case you want a log file the smae full path as your app is
	BEWARE! be very carefull with relative file names!
	*/
	inline std::string app_to_log_file_name(
		std::string_view full_exe_path,
		char const* const suffix = ".log"
	)
	{
#ifdef _MSC_VER
		constexpr auto  DBJ_PATH_DELIM = '\\';
#else
		constexpr auto  DBJ_PATH_DELIM = '/';
#endif
		//auto pos_ = full_exe_path.find_last_of(DBJ_PATH_DELIM);
		//if (pos_ == full_exe_path.npos) return {};
		//auto basename = full_exe_path.substr(pos_);
		//return std::string{"."}.append( basename.data() ).append(suffix);
		return std::string{ full_exe_path.data() }.append(suffix);
	}

	 typedef enum  {
		MT = 1 , /* set to Multi Threaded */
		VT100_CON = 2, /* specificaly switch on the VT100 console mode */
		LOG_FROM_APP_PATH = 4, /* if app full path is given  use it to obtain log gile name*/
		FILE_LINE_OFF = 8 /* do not show file line on every log line */
	} SETUP ;

	/*
	 ---------------------------------------------------------------------------------------------------------
	usage
	   void main() {
		dbj::simplelog::setup(__argv[0]);
	   }
	*/

	inline bool setup(int setup = SETUP::VT100_CON, const char* app_full_path = nullptr) 
	{
		if (( int(setup) & int(SETUP::VT100_CON)) != 0) {
			enable_vt_mode();
		}

		if (( int(setup) & int(SETUP::MT) ) != 0) {
			log_set_udata(mt::global_log_protector_ptr);
			log_set_lock(mt::protector_function);
		}

		// caller does not want any kind of local log file
		if (app_full_path == nullptr) return true;

		string log_file_name {};
		if ((int(setup) & int(SETUP::LOG_FROM_APP_PATH)) != 0) {
			log_file_name = dbj::simplelog::app_to_log_file_name(app_full_path);
		}
		else {
			log_file_name = app_full_path;
		}
			auto [file_handle, status] = dbj::simplelog::FH::assure(log_file_name);

			if (!file_handle) {
				perror(status->data());
				return false;
			}

			log_set_fp(file_handle->file_ptr());
				return true;
	}

} // namespace dbj::simplelog;

#endif //  __cplusplus

#ifndef __cplusplus
#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#else
#define log_trace(...) dbj::simplelog::log_log(dbj::simplelog::LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) dbj::simplelog::log_log(dbj::simplelog::LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  dbj::simplelog::log_log(dbj::simplelog::LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  dbj::simplelog::log_log(dbj::simplelog::LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) dbj::simplelog::log_log(dbj::simplelog::LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) dbj::simplelog::log_log(dbj::simplelog::LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#endif // !__cplusplus



#endif

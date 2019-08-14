#pragma once

namespace dbj::win32 {

	inline int last_win32_error() noexcept
	{
		struct last final
		{
			last() noexcept : error_(::GetLastError()) {}
			~last() { ::SetLastError(0); }
			int operator() () const noexcept { return error_; }
		private:
			mutable int error_{};
		};

		int last_error_ = (last{})();
		return last_error_;
	};

	// return instance of std::system_error
	// which for MSVC STL delivers win32 last error message
	// by calling what() on it
	inline auto system_error_instance()
		->  std::system_error
	{
		using namespace std;
		return system_error(error_code(last_win32_error(), system_category()));
	}

	// Returns the last Win32 error message
	// with optional prefix
	inline  typename ::dbj::chr_buf::narrow_type // ::dbj::chr_buf::yanb 
		get_last_error_message(
			std::string_view prompt = std::string_view{}
		)
	{
		typename ::dbj::chr_buf::narrow_type buffer_;
		//Get the error 
		auto syserr = system_error_instance();
		const char* sys_err_msg = syserr.what();

		if (!prompt.empty())
		{
			//use std::string only and if needed
			std::string str_(prompt); str_.append(sys_err_msg);

			dbj::chr_buf::core::assign(buffer_, str_.c_str());
		}
		else {
			dbj::chr_buf::core::assign(buffer_, sys_err_msg);

		}
		return  buffer_;
	}

	/*
Bellow is WIN32 API, adjusted for standard C++ use
dbj::win32::string_compare()
locale sensitive so to be used for "UI string comparisons",
ui sorting, and a such
retval is standard -1,0,1 triplet
*/
	inline int string_compare(LPCTSTR str1, LPCTSTR str2, unsigned char ignore_case)
	{
		int rez = CompareStringEx(
			LOCALE_NAME_USER_DEFAULT,
			ignore_case == 1 ? LINGUISTIC_IGNORECASE : NORM_LINGUISTIC_CASING,
			str1,
			-1,
			str2,
			-1,
			NULL, NULL, 0
		);
		switch (rez) {
		case CSTR_LESS_THAN: rez = -1; break;
		case CSTR_EQUAL: rez = 0; break;
		case CSTR_GREATER_THAN: rez = 1; break;
		}
		return rez;
	}


} // dbj::core

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"

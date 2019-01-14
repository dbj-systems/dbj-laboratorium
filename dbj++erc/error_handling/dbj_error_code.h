#pragma once
#include <exception>
#include <string>
#include <string_view>
/*
DBJ++ error concept
-----------------------------

Bjarne suggests to both return "things" and throw exceptions.
Also we will have catch almost nowhere but inside the main().

Since std error codes and str error conditions are overengineered mess
and std expected is adding into that mess we will almost follow
how std ios is using this mess.

To understand a lot about ::std exception and error observe very carefully
the ios::base_failure exception positioning as explained here
https://en.cppreference.com/w/cpp/io/ios_base/failure

Author of std error codes concept has dissapeared, but we will follow
his abruptly stoped posts on the subject

http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-4.html
*/
namespace dbj::err 
{
	using namespace ::std::string_view_literals;
	using ::std::string;
	using ::std::wstring;

	// error codes
	enum class codes {
		ok = 100,
		bad_argument = 101,
		bad_index,
		bad_length,
		bad_type
	}; // codes

	class common_category
		: public std::error_category
	{
	public:
		virtual const char* name()  const noexcept {
			return "dbj++common";
		}


		virtual std::string message(int ev) const noexcept {
			/*
			from here we should obviously call some message map
			*/
			switch ((codes)ev) {
			case codes::bad_argument:
				return "Bad Argument";
			case codes::bad_index:
				return "Bad index";
			case codes::bad_length:
				return "Bad length";
			case codes::bad_type:
				return "Bad type";
			default:
				return "Unknown error";
			}
		}
	};

	inline const std::error_category& category()
	{
		static common_category category_;
		return category_;
	}

	inline std::error_code make_error_code(codes e)
	{
		return std::error_code(
			static_cast<int>(e),
			category());
	}

	inline  std::error_condition make_error_condition(codes e)
	{
		return std::error_condition(
			static_cast<int>(e),
			category());
	}
} // dbj::err

namespace std
{
	template <>
	struct is_error_code_enum<::dbj::err::codes>
		: public true_type {};
}
#pragma endregion

/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"
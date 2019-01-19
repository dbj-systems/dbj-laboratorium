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

Author of std error codes concept has 'dissapeared', but we will follow
his abruptly stoped posts on the subject

http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-4.html
*/
namespace dbj::err 
{
	using namespace ::std::string_view_literals;
	using ::std::string;
	using ::std::wstring;

#pragma region utils

#ifdef _MSC_VER

	// (c) dbjdbj 2019 JAN
	struct last_win_err final
	{
		~last_win_err() {
			// make sure this is done
			// windows lore says this is required
			::SetLastError(0);
		}

		// make error code from win32 err int code 
		// POLICY: last WIN32 error is obtained on each call
		operator std::error_code() const noexcept {
			return std::error_code(::GetLastError(), std::system_category());
		}

	};

	inline std::error_code last_win_ec() {
		return static_cast<std::error_code>(last_win_err{});
	}
#endif // _MSC_VER

#pragma endregion

	// error codes
	enum class dbj_err_code {
		bad_argument = 101,
		bad_index,
		bad_length,
		bad_type
	}; // codes

	class common_category final
		: public std::error_category
	{
	public:
		[[nodiscard]] virtual const char* name()  const noexcept {
			return "dbj++common";
		}


		[[nodiscard]] virtual std::string message(int ev) const noexcept {
			/*
			from here we should obviously call some message map
			*/
			switch (static_cast<dbj_err_code>(ev)) {
			case dbj_err_code::bad_argument:
				return "Bad Argument";
			case dbj_err_code::bad_index:
				return "Bad index";
			case dbj_err_code::bad_length:
				return "Bad length";
			case dbj_err_code::bad_type:
				return "Bad type";
			default:
				return "Unknown error";
			}
		}

// OPTIONAL: Allow generic error conditions 
// to be compared to this error domain aka category
		[[nodiscard]] virtual std::error_condition
			default_error_condition(int c) 
				const noexcept override final
		{
			switch (static_cast<dbj_err_code>(c))
			{
			case dbj_err_code::bad_argument:
			case dbj_err_code::bad_length:
			case dbj_err_code::bad_type:
				return make_error_condition(std::errc::invalid_argument);
			case dbj_err_code::bad_index:
				return make_error_condition(std::errc::result_out_of_range);
			default:
				// No mapping 
				return std::error_condition(c, *this);
			}
		}
	};

	inline const std::error_category& get_common_category()
	{
		static common_category category_;
		return category_;
	}

	inline std::error_code make_error_code(dbj_err_code e)
	{
		return std::error_code(
			static_cast<int>(e),
			get_common_category());
	}

	inline  std::error_condition make_error_condition(dbj_err_code e)
	{
		return std::error_condition(
			static_cast<int>(e),
			get_common_category());
	}
} // dbj::err

namespace std
{
	template <>
	struct is_error_code_enum<::dbj::err::dbj_err_code>
		: public true_type {};
}
#pragma endregion

/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"
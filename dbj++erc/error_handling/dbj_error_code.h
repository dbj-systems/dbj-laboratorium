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

	// error codes
	enum class common_codes {
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
			switch (static_cast<common_codes>(ev)) {
			case common_codes::bad_argument:
				return "Bad Argument";
			case common_codes::bad_index:
				return "Bad index";
			case common_codes::bad_length:
				return "Bad length";
			case common_codes::bad_type:
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
			switch (static_cast<common_codes>(c))
			{
			case common_codes::bad_argument:
			case common_codes::bad_length:
			case common_codes::bad_type:
				return make_error_condition(std::errc::invalid_argument);
			case common_codes::bad_index:
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

	inline std::error_code make_error_code(common_codes e)
	{
		return std::error_code(
			static_cast<int>(e),
			get_common_category());
	}

	inline  std::error_condition make_error_condition(common_codes e)
	{
		return std::error_condition(
			static_cast<int>(e),
			get_common_category());
	}
} // dbj::err

namespace std
{
	template <>
	struct is_error_code_enum<::dbj::err::common_codes>
		: public true_type {};
}
#pragma endregion

/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"
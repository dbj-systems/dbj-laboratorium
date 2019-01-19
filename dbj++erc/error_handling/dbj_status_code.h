#pragma once
#include "pch.h"

namespace dbj::err {

/*
Overload journal 141 (October 2017) includes an article by Ralph McArdell
titled "C++11 (and beyond) Exception Handling," which presents a set of
proposed "best practices" for using <dbj_status_code>.
These best practices basically align with Arthur O'Dwyer's best practices.
A notable difference is that McArdell recommends
constructing the "no error" error_code value differently:

  class ErrCategory : std::error_category { ... };
enum ErrCode { success = 0, failure1 = 1, failure2 = 2 };
template<> struct std::is_error_code_enum : std::true_type {};
inline std::error_code make_error_code(ErrCode e) {
	static const ErrCategory c;
	return std::error_code((int)e, c);
}

std::error_code McArdellNoError() {
	return ErrCode{};  // that is, {0, ErrCategory}
}
 
 default category is std::dbj_status_code
 default error code using "by convention" as 
 "no error here" is a mistake many have made
 for example AZURE API
 inline std::error_code error_code_ok{};

*/
	enum class dbj_status_code {
		ok = 0, warning = 1, info = 2, error = 3,
	};
}


namespace std {
	// must mark it as error_code_enum
	// while in std space
	template<>
	struct std::is_error_code_enum<dbj::err::dbj_status_code> : std::true_type {};
}

namespace dbj::err {

	namespace inner {
		class dbj_status_category final : public std::error_category
		{
			[[nodiscard]]
			const char * name() const noexcept {
				return "dbj_status";
			}

			[[nodiscard]]
			std::string message(int errval_) const
			{
				switch (static_cast<dbj_status_code>(errval_)) {
				case dbj_status_code::ok: return { "Ok" };
				case dbj_status_code::warning: return { "Warning" };
				case dbj_status_code::info:return { "Info" };
				case dbj_status_code::error:return { "Error" };
				default:
					return { "Unknown" };
				}
			}
			// OPTIONAL: Allow generic error conditions 
			// to be compared to this category
			[[nodiscard]] virtual std::error_condition
				default_error_condition(int c) const noexcept override final
			{
				switch (static_cast<dbj_status_code>(c))
				{
				case dbj_status_code::ok:
				case dbj_status_code::warning:
				case dbj_status_code::info:
				case dbj_status_code::error:
				default:
					// No mapping 
					return std::error_condition(c, *this);
				}
			}
		};
	} // ns inner

	// Declare a global function returning a static instance 
	// of the custom category
	inline inner::dbj_status_category const &
		make_dbj_status_category ()
	{
		// dbj: this is resilient in presence of
		// multiple threads
		static inner::dbj_status_category dsc_ ;
		return dsc_;
	}

	inline std::error_code make_error_code(dbj_status_code e_) {
		return std::error_code{
			(int)e_, make_dbj_status_category()
		};
	}

	// the one and only 
	inline std::error_code dbj_universal_ok
		= make_error_code(dbj_status_code::ok);
}

/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"
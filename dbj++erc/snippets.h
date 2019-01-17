#include "pch.h"
#include "./error_handling/dbj_status_code.h"

//to be renamed to dbj::erc
namespace dbj_erc {

	using ::dbj::console::print;

	// here we return "system ok" 
	[[nodiscard]]
	auto very_complex_operation()
		noexcept
	{
		std::error_code ec = DBJ_TEST_ATOM(
			std::make_error_code(std::errc::address_family_not_supported)
		);

		std::error_condition en = DBJ_TEST_ATOM(
			ec.default_error_condition()
		);

		return std::pair{ ::dbj::err::dbj_universal_ok , 42 };
	}

	namespace {
		using namespace ::dbj::err;

		// we might return this "everywhere" we need to signal OK return
		inline std::error_code const & dbj_ok = ::dbj::err::dbj_universal_ok;

		// here we return specific dbj ok
		[[nodiscard]]
		auto very_complex_dbj_operation
		(bool whatever = true )
			noexcept
		{
			if ( whatever )
				return std::make_pair(dbj_ok, 13);
			// else
			return std::make_pair(
				dbj::err::make_error_code(
					::dbj::err::common_codes::bad_argument),
				-1);
		}


		inline void dbj_error_system_by_the_book ()
		{
			using ::dbj::console::print;

			if (auto[e, v] = very_complex_operation(); e == dbj_universal_ok) {
				print("\n\nSYSTEM OK, return value is: ", v);
			}

			if (auto[e, v] = very_complex_dbj_operation()
				; e == dbj_ok)
			{
				print("\n\nDBJ OK, return value is: ", v);
			}
			else {
				print("\n\ndbj NOT OK, return status is ", e.message(),
					", return value is: ", v);
			}

			[[nodiscard]]
			auto server_side_handler = []
			(std::error_code& ec)
				noexcept
			{
				ec = common_codes::bad_argument;
			};

			std::error_code ec;
			server_side_handler(ec);
			if (ec == common_codes::bad_argument)
			{
				DBJ_TEST_ATOM(ec.message());
				DBJ_TEST_ATOM(ec.value());

				auto cond = ec.default_error_condition();
				DBJ_TEST_ATOM(cond.message());
				DBJ_TEST_ATOM(cond.value());
			}

		}
	}

	inline void win32_system_specific_errors()
	{
		using ::dbj::console::print;

		auto print_last_win32_error = []() {
			print("\n\nWIN32 Error:\n",
				last_win_ec(),
				"\n");
		};

		try {
			std::string f;
			auto mandatory_retval = f.at(13);
		}
		catch (const std::exception & e) {
			DBJ_TEST_ATOM(e);
		}

		char lpBuffer[64]{};
		// provoke system error
		DWORD DBJ_MAYBE(rv) = GetEnvironmentVariable(
			LPCTSTR("whatever_non_existent_env_var"),
			LPTSTR(lpBuffer),
			DWORD(64)
		);
		print_last_win32_error();
	}

	void why_not() 
	{
		try {
			// assignment behaviour makes the
			// error code
			std::error_code info_	= dbj::err::dbj_status_code::info;
			std::error_code ok_		= dbj::err::dbj_status_code::ok;
			auto same_cat_ = info_.category() == ok_.category();
			// just throw the bastard :)
			throw info_;
		}
		catch ( std::error_code ec ) 
		{
			// can do all the usual comparisons and
			// interogations here
			if (ec == std::errc::not_enough_memory) 
				print("\nApparently there is no enough memory?");

			DBJ_TEST_ATOM(ec);

		} catch (std::error_condition ecn) {
			DBJ_TEST_ATOM(ecn);
		}
	}

	void p1095_tests() {
		auto[v, e] = ::dbj_erc::safe_divide(1, 2);
		DBJ_TEST_ATOM(v);
		DBJ_TEST_ATOM(e);
	}

} // dbj_erc

// use of DBJ TESTING FWK is "moved out"
// so the users can easily opt out
// and call directly or whatever
DBJ_TEST_UNIT(one)
{
	::dbj_erc::p1095_tests();
	::dbj_erc::why_not();
	::dbj_erc::dbj_error_system_by_the_book();
	::dbj_erc::win32_system_specific_errors();
}


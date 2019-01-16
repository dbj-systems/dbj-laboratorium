#include "pch.h"
#include "dbj_status_code.h"

//to be renamed to dbj::erc
namespace dbj_erc {

	struct last_win_err final
	{
		~last_win_err() {
			// make sure this is done
			::SetLastError(0);
		}

		// make error code from win32 err int code 
		operator std::error_code() const noexcept {
			return std::error_code(::GetLastError(), std::system_category());
		}

	};

	inline std::error_code last_win_ec() {
		return static_cast<std::error_code>(last_win_err{});
	}

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
	/*
		the C++23 future 

		int safe_divide(int i, int j) fails(arithmetic_errc) {
		if (j == 0)
		return failure(arithmetic_errc::divide_by_zero);
		if (i == INT_MIN && j == -1)
		return failure(arithmetic_errc::integer_divide_overflows);
		if (i % j != 0)
		return failure(arithmetic_errc::not_integer_division);
		else return i / j;
		}

		double caller(double i, double j, double k) throws {
		return i + safe_divide(j, k);
		}

		the immediate dbj++erc

		we use "dbj_" prefix so when C++23 arrives we can easily search/replace
		*OR* run without changes since we will not have a clash with
		new C++23 keywords
	*/
	// future 'throws' function marker
	// just nothing for the time being
	#define dbj_throws
	// P1095 fails used "now"
	// declares the return value pair type
	// to which failure/succes making inside 
	// the same function has to conform
	#define dbj_fails(vt,et) -> std::pair<vt,et>
    #define dbj_erc_retval auto

	template<typename T>
	auto failure (T v, std::errc e_) { 
		return std::pair{ v, std::make_error_code(e_) };
	}

	template<typename T>
	auto failure (T v, std::error_code e_) {
		return std::pair{ v, e_ };
	}

	template<typename T>
	auto failure (T v, std::error_condition en_) {
		return std::pair{ v, std::make_error_code(en_) };
	}

	template<typename T>
	auto succes (T v) {
		return std::pair{ v, dbj_universal_ok };
	};

	inline dbj_erc_retval safe_divide(int i, int j) 
		dbj_fails(int, std::error_code)
	{
		// note: failure/success making has to conform to
		// the fails declaration, or  the code
		// won't compile
		// note: std::errc are completely arbitrary here
		if (j == 0)
			return failure(0, std::errc::invalid_argument);
		if (i == INT_MIN && j == -1)
			return failure(0,std::errc::invalid_argument);
		if (i % j != 0)
			return failure(0,std::errc::invalid_argument);
		else 
			return succes((int)(i / j));
	}

} // dbj_erc


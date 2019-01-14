#include "pch.h"

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

	inline std::error_code last_win_ec	(	) {
		return static_cast<std::error_code>(last_win_err{});
	}



	// default category is std::system_error
	inline std::error_code error_code_ok{};

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

		return std::pair{ error_code_ok, 42 };
	}

	namespace {
		using namespace ::dbj::err;

		// we might return this "everywhere" we need to signal OK return
		inline std::error_code dbj_ok
		{
			make_error_code(codes::ok)
		};

		// here we return specific dbj ok
		[[nodiscard]]
		auto very_complex_dbj_operation
		(::dbj::err::codes request = ::dbj::err::codes::ok)
			noexcept
		{
			if (request == ::dbj::err::codes::ok)
				return std::make_pair(dbj_ok, 13);
			// else
			return std::make_pair(
				dbj::err::make_error_code(request),
				-1);
		}


		inline void dbj_error_system_by_the_book ()
		{
			using ::dbj::console::print;

			if (auto[e, v] = very_complex_operation(); e == error_code_ok) {
				print("\n\nSYSTEM OK, return value is: ", v);
			}

			if (auto[e, v] = very_complex_dbj_operation(::dbj::err::codes::bad_argument)
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
				ec = codes::bad_argument;
			};

			std::error_code ec;
			server_side_handler(ec);
			if (ec == codes::bad_argument)
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

} // dbj_erc


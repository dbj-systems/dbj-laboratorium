#include "pch.h"
// #include "./error_handling/dbj_err.h"

//to be renamed to dbj::erc
namespace dbj_erc {

	using ::dbj::console::print;
	using namespace ::dbj::err;

	// here we return "system ok" 
	[[nodiscard]]
	inline auto very_complex_operation()
		noexcept
	{
		std::error_code ec = DBJ_TEST_ATOM(
			std::make_error_code(std::errc::address_family_not_supported)
		);

		std::error_condition en = DBJ_TEST_ATOM(
			ec.default_error_condition()
		);

		return std::pair{ dbj_universal_ok , 42 };
	}

		// here we return specific dbj ok
		[[nodiscard]]
		inline auto very_complex_dbj_operation
		(bool whatever = true )
			noexcept
		{
			if ( whatever )
				return std::make_pair(dbj_universal_ok, 13);
			// else
			return std::make_pair(
				make_error_code(
					dbj_err_code::bad_argument),
				-1);
		}


		inline void dbj_error_system_by_the_book ()
		{
			if (auto[e, v] = very_complex_operation(); e == dbj_universal_ok) {
				print("\n\nSYSTEM OK, return value is: ", v);
			}

			if (auto[e, v] = very_complex_dbj_operation()
				; e == dbj_universal_ok)
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
				ec = dbj_err_code::bad_argument;
			};

			std::error_code ec;
			server_side_handler(ec);
			if (ec == dbj_err_code::bad_argument)
			{
				DBJ_TEST_ATOM(ec.message());
				DBJ_TEST_ATOM(ec.value());

				auto cond = ec.default_error_condition();
				DBJ_TEST_ATOM(cond.message());
				DBJ_TEST_ATOM(cond.value());
			}

		}

	inline void win32_system_specific_errors()
	{
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

	inline void why_not() 
	{
		try {
			std::error_code ecodes[]{ 
				// condition to code
				std::make_error_code(std::errc::not_enough_memory) ,
				// simple assignment makes the error code
				dbj_status_code::info,
				dbj_status_code::ok };

			// test is_dbj_err
			DBJ_TEST_ATOM( is_dbj_err(ecodes[0]));
			DBJ_TEST_ATOM( is_dbj_err(ecodes[1]));
			DBJ_TEST_ATOM( is_dbj_err(ecodes[2]));
			// just throw the bastard :)
			throw ecodes[1];
		}
		catch ( std::error_code ec ) 
		{
			// all the usual tests 
			if (ec == std::errc::not_enough_memory) 
				print("\nApparently there is no enough memory?");
			else
			if (ec == dbj_status_code::ok)
				print("\nSome dbj++ api sent ok signal");
			else
			if (ec == dbj_status_code::info)
				print("\nSome dbj++ api sent info signal");
			// and so on
			DBJ_TEST_ATOM(ec);
		} catch (std::error_condition ecn) {
			DBJ_TEST_ATOM(ecn);
		}
	}
		// the usage of P1095 described features
		// used here
		[[nodiscard]]
		inline dbj_erc_retval safe_divide(int i, int j)
			// effectively declare the return type 
			dbj_fails(int, std::error_code)
		{
			// note: failure/success making has to conform to
			// the fails declaration, or  the code
			// won't compile
			if ( (j == 0)
			   ||(i == INT_MIN && j == -1)
			   ||(i % j != 0) )
				return failure(0, std::errc::invalid_argument);
			else
#if 0
			   // INFO RETURN
  			   return failure((int)(i / j), dbj_status_code::info);
#endif
			return succes(i / j);
		}

	inline void p1095_tests() {
		if (auto[v, e] = ::dbj_erc::safe_divide(4, 2); e) {
			DBJ_TEST_ATOM(v);
			DBJ_TEST_ATOM(e);
		}
	}

} // dbj_erc

namespace detail {
	/*
	return array reference to the
	native array inside std::array
	*/
	template<typename T, size_t N,
		typename ARR = std::array<T, N>, /* std::array */
		typename ART = T[N],    /* the native array */
		typename ARF = ART & ,  /* reference to it */
		typename ARP = ART * >  /* pointer   to it */
		constexpr inline
		ARF
		internal_array_reference(const std::array<T, N> & arr)
	{
		return *(ARP)
			const_cast<typename ARR::pointer>
			(arr.data());
	}
};

// use of DBJ TESTING FWK is "moved out"
// so the users can easily opt out
// and call directly or whatever
DBJ_TEST_UNIT(one)
{
	using namespace ::dbj_erc;
		p1095_tests();
		why_not();
		dbj_error_system_by_the_book();
		win32_system_specific_errors();
}


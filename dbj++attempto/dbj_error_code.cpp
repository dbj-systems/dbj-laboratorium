#include "pch.h"
#include <fstream>

/* 
it is a standard way how to do it so
we will copy the design of std:: ios error coding and signaling
*/
#include <ios>

#ifndef _HAS_CXX17
#error C++17 required
#endif

DBJ_TEST_SPACE_OPEN(how_to_use_system_error)

DBJ_TEST_UNIT(dbj_error_system_by_the_book)
{
	auto server_side_handler = [](
		std::error_code& ec)
	{
		ec = ::dbj::err::codes::bad_argument;
	};

	std::error_code ec;
	server_side_handler(ec);
	if (ec == ::dbj::err::codes::bad_argument)
	{
		DBJ_TEST_ATOM(ec.message());
		DBJ_TEST_ATOM(ec.value());

		auto cond = ec.default_error_condition();
		DBJ_TEST_ATOM(cond.message());
		DBJ_TEST_ATOM(cond.value());
	}

}
	 

DBJ_TEST_UNIT(win32_system_specific_errors) 
{
	using ::dbj::console::print;

	std::ifstream f("doesn't exist");
	try {
		f.exceptions(f.failbit);
	}
	catch (const std::ios_base::failure& e) {
		print("Caught an ios_base::failure.\n");
		if (e.code() == std::io_errc::stream) {
			print("The error code is std::io_errc::stream\n");
			print(e.code());
		}
	}
	/*
	we can use ::std artefacts on MSVC platform to get system error messages
	*/
	char lpBuffer[64]{};
	// provoke system error
	DWORD DBJ_MAYBE(rv) = GetEnvironmentVariable(
		LPCTSTR("whatever_non_existent_env_var"),
		LPTSTR(lpBuffer),
		DWORD(64)
	);
	auto last_win32_err = ::GetLastError();
	// make error code from win32 err, int code 
	auto ec = std::error_code(last_win32_err, std::system_category());
	dbj::console::print("\n\n",ec, "\n");
}

DBJ_TEST_SPACE_CLOSE


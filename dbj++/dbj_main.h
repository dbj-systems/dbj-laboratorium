#pragma once
/*
her we offer a "proper" main that deals with uncaught exceptions
in a standard way
*/

// #define DBJ_WMAIN_USED

#ifdef DBJ_WMAIN_USED

#if defined(_WIN32) || defined (_WIN64)

#endif

namespace dbj 
{
	namespace final_for_main {
		/*
		this works only if and when called from inside the catch block
		*/
		inline void handle_eptr(std::exception_ptr eptr) // passing by value is ok
		{
			using namespace ::dbj::console;
			try {
				if (eptr) {
					std::rethrow_exception(eptr);
				}
			}
			catch (const std::exception& x_) {
				print("\nstd exception: ", x_.what());
			}
		}

		inline void final_exceptions_processor()
		{
			using namespace ::dbj::console;

			std::exception_ptr eptr;

			print("\n", app_base_name, " -- Exception caught! -- ", painter_command::bright_red);

			try {
				throw;
			}
			catch (std::system_error & x_) {
				print("system error: ", x_.code());
			}
			catch (::std::exception & x_) {
				print("std Exception, ", x_.what());
			}
			catch (...) {
				eptr = std::current_exception(); // final capture
			}
			handle_eptr(eptr);
			print(painter_command::text_color_reset, "\n");
		}
	} // final_for_main

} // dbj

///////////////////////////////////////////////////////////////////////////////////
//
// users muste define this if DBJ_WMAIN_USED is defined

extern inline void dbj_program_start(
	int , /* argc */
	const wchar_t * [], /* argv */
	const wchar_t *[] /* envp */
);

#if defined(UNICODE) || defined(_UNICODE)
int wmain(const int argc, const wchar_t *argv[], const wchar_t *envp[])
#else
#error "What could be the [t]reason this is not an UNICODE build?"
int main(int argc, char* argv[], char *envp[])
#endif
{
	auto main_worker = [&]() {
		using namespace ::dbj::console;
		namespace  ffm = ::dbj::final_for_main;
		try {
			dbj_program_start(argc, argv, envp);
		}
		catch (std::error_code ec) {
			print("\n\nAn error_code caught in \n\t",
				painter_command::bright_red,
				ec,
				painter_command::text_color_reset
			);
		}
		catch (...) {
			ffm::final_exceptions_processor();
		}

	};

	(void)std::async(std::launch::async, [&] {
		main_worker();
	});

	exit(EXIT_SUCCESS);
}

#endif // DBJ_WMAIN_USED

/* inclusion of this file defines the kind of a licence used */
#include "dbj_license.h"

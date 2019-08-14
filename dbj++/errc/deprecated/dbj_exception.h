#pragma once
#ifdef DBJ_ERR_STANDARD_EXCEPTION
#include "dbj_error_code.h"
namespace dbj::err 
{
	using namespace ::std;
	using namespace ::std::string_view_literals;
	using ::std::string;
	using ::std::wstring;

#pragma warning(push)
#pragma warning(disable: 4577) // 'noexcept' used with no exception handling mode specified

	// base of all dbj++ exceptions
	// holds one of dbj::err::codes
	// made such so that standard C++ 
	// usage of dbj++ is "normal" in presence of
	// multiple libs
	class dbj_exception : public std::system_error
	{
		using base = std::system_error;

	public:

		dbj_exception(std::error_code ec) :base(ec) {
		}
/*
 Constructs with error code ec and explanation string what_arg. 
 The string returned by what() is guaranteed to contain what_arg as a substring.
 */
		dbj_exception(std::error_code ec, const std::string& what_arg)
			: base(ec,what_arg)
		{}
		dbj_exception(std::error_code ec, const char* what_arg)
			: base( ec, what_arg )
		{}
/* 
Constructs with underlying error code ev and associated error category ecat.
*/
		dbj_exception(int ev, const std::error_category& ecat)
			: base(ev,ecat)
		{}
/*
Constructs with underlying error code ev, associated error category ecat 
and explanatory string what_arg. 
The string returned by what() is guaranteed to contain what_arg as a substring.
*/
		dbj_exception(int ev, const std::error_category& ecat, const std::string& what_arg)
			: base(ev,ecat,what_arg)
		{}
		dbj_exception(int ev, const std::error_category& ecat, const char* what_arg)
			: base(ev, ecat, what_arg)
		{}

	private:

		friend std::wstring wwhat(const dbj_exception & x_)
		{
			std::wstring wwstring
				= ::dbj::nano::transform_to<wstring>(std::string(x_.what()));
			return wwstring;
		}

		// friend for dbj console print system
		// print exceptions and also color the output red 
		friend 	void out ( const dbj_exception & x_) 
		{
			using ::dbj::console::PRN;
			out(::dbj::console::painter_command::bright_red);
			PRN.wchar_to_console(L"\ndbj::exception\n");
			PRN.wchar_to_console(wwhat(x_).c_str());
			PRN.wchar_to_console(L"\ndbj::exception error_code\n");
			PRN.wchar_to_console(L"\nCategory: ");
			PRN.char_to_console( x_.code().category().name() );
			PRN.wchar_to_console(L"\nvalue: ");
			PRN.printf( "%d", x_.code().value() );
			PRN.wchar_to_console(L"\nMessage: ");
			PRN.printf( "%s", x_.code().message().c_str() );
			out(::dbj::console::painter_command::text_color_reset);
		}
	};

#pragma warning(pop)
} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"
#endif // DBJ_ERR_STANDARD_EXCEPTION

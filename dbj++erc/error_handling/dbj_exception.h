#pragma once

#include "dbj_error_code.h"

namespace dbj 
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
	class exception : public std::exception
	{
		using parent_type = std::exception;
		mutable error_code err_code_{};
	public:

		explicit exception(const char * const message_,
			const error_code& erc_ = make_error_code(dbj::err::codes::ok))
			: parent_type(message_)
			, err_code_(erc_)
		{			
			DBJ_VERIFY(nullptr != message_);
		}

		exception(wchar_t const* const message_, 
			const error_code& erc_ = make_error_code(dbj::err::codes::ok)
		) noexcept
			: parent_type( 
				nano::transform_to<string>(wstring(message_)).c_str()
			)
			, err_code_(erc_)
		{
			DBJ_VERIFY(nullptr != message_);
		}

		template< typename C >
		exception(::std::basic_string_view<C> msg_,
			const error_code& erc_ = make_error_code(dbj::err::codes::ok)
		)
			: parent_type(nano::transform_to<string>(msg_).c_str())
			, err_code_(erc_)
		{
			DBJ_VERIFY(msg_.size());
		}

		template< typename C >
		exception(const std::basic_string<C> & msg_,
			const error_code& erc_ = make_error_code(dbj::err::codes::ok))
			: parent_type(nano::transform_to<string>(msg_).c_str())
			, err_code_(erc_)
		{
			DBJ_VERIFY(msg_.size());
		}

		std::error_code code() const noexcept
		{
			return this->err_code_;
		}

		wchar_t const * wwhat() const
		{
		  this->wwstring 
			  = ::dbj::nano::transform_to<wstring>(string(this->what()));
		  return this->wwstring.c_str();
		}
	private:
		mutable wstring wwstring{};

		// friend for dbj console print system
		// print exceptions and also color the output red 
		friend 	void out ( const dbj::exception & x_) 
		{
			using ::dbj::console::PRN;
			out(::dbj::console::painter_command::bright_red);
			PRN.wchar_to_console(L"\ndbj::exception\n");
			PRN.wchar_to_console(x_.wwhat());
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
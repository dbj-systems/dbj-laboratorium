#pragma once
#include "../win/dbj_win32.h"
#include "dbj_console_fwd.h"
#include "dbj_console_painter.h"
#include <io.h>
#include <fcntl.h>

namespace dbj {
	extern inline bool console_is_initialized() ;
}

namespace dbj::console {

#pragma region WideOut
	/*
	The "core of the trick"

	SetConsoleOutputCP(CP_UTF8);
	_setmode(_fileno(stdout), _O_U8TEXT);
	wprintf(L"UTF-8 display: %s\r\n", uname);   //uname is a wchar_t*

	Windows "native" unicode is UTF-16
	Be warned than proper implementation of UTF-8 related code page did not happen
	before W7 and just *perhaps* it is full on W10
	See :	http://www.dostips.com/forum/viewtopic.php?t=5357
	Bellow is not FILE * but HANDLE based output.
	It also uses #define CP_UTF8 65001, as defined in winnls.h
	This two are perhaps why this almost always works.

	https://msdn.microsoft.com/en-us/library/windows/desktop/dd374122(v=vs.85).aspx

	Even if you get your program to write UTF16 correctly to the console,
	Note that the Windows console isn't Unicode friendly and may just show garbage.
	*/
	struct WideOut final : public IConsole
	{
		mutable		HANDLE output_handle_;
		mutable		UINT   previous_code_page_;
		mutable		UINT	code_page_{};

	public:

		static constexpr CODE DEFAULT_CODEPAGE_ = CODE::page_65001 ;

		WideOut(CODE CODEPAGE_ = DEFAULT_CODEPAGE_) noexcept
			: code_page_((UINT)CODEPAGE_)
			, output_handle_(::GetStdHandle(STD_OUTPUT_HANDLE))
			, previous_code_page_(::GetConsoleOutputCP())
		{
			//this->output_handle_ = ::GetStdHandle(STD_OUTPUT_HANDLE);
			_ASSERTE(INVALID_HANDLE_VALUE != this->output_handle_);
			// previous_code_page_ = ::GetConsoleOutputCP();
			_ASSERTE(0 != ::SetConsoleOutputCP(code_page_));
			/*			TODO: GetLastError()			*/
			// apparently for a good measure one has to do this too ...
			::SetConsoleCP(code_page_);
			// we do NOT use file handlers but
			// this is REALLY important to do
			_setmode(_fileno(stdout), _O_U8TEXT);
			// after this guess the right font and you are ok ;)
		}
		// no copying
		WideOut(const WideOut & other) = delete;
		WideOut & operator = (const WideOut & other) = delete;
		// we need this one so we can pass the singleton instance out
		WideOut(WideOut && other) = default;
		WideOut & operator = (WideOut && other) = default;

		~WideOut()
		{
			auto DBJ_MAYBE(rezult) = ::SetConsoleOutputCP(previous_code_page_);
			// apparently for a good measure one has to do this too ...
			::SetConsoleCP(previous_code_page_);
			_ASSERTE(0 != rezult);
		}

		/* what code page is used */
		const unsigned code_page() const noexcept override { return this->code_page_; }
		/* out__ is based on HANDLE and std::wstring */
		HANDLE handle() const override  { 

			DBJ_VERIFY(output_handle_ != INVALID_HANDLE_VALUE);
#ifdef _DEBUG
			DWORD lpMode{};
			DBJ_VERIFY(0 != GetConsoleMode(output_handle_, &lpMode));
#endif
				return this->output_handle_;
		}
    	// from --> to, must be a sequence
		// this is the *fastest* method
		void out( const wchar_t * from,  const wchar_t * to) const override
		{
#ifndef _DEBUG
			static
#endif // !_DEBUG
			const HANDLE output_h_ =  this->handle();

			_ASSERTE(output_h_ != INVALID_HANDLE_VALUE);
			_ASSERTE(from != nullptr);
			_ASSERTE(to != nullptr);
			_ASSERTE(from != to);

			std::size_t size = std::distance(from, to);
			_ASSERTE( size > 0 );

			// this is *crucial*
			// otherwise ::WriteConsoleW will fail
			// in a debug builds on unpredicatble
			// and rare ocasions
			// the solution appers to be to
			// effectively set the last error to 0
			(void)::SetLastError(0);

			auto retval = ::WriteConsoleW
			(
				output_h_,
				from,
				static_cast<DWORD>(size), NULL, NULL
			);
			DBJ_VERIFY(retval != 0);
	} // out

	/* as dictated by the interface implemented */
	void out(const std::wstring_view wp_) const override
	{
		this->out(wp_.data(), wp_.data() + wp_.size());
	}

		private:
		/*
		here we hide the single application wide 
		IConsole instance maker
		*/
		static WideOut & instance( 
			CODE_PAGE const & code_page = default_code_page
		)
		{
			static WideOut single_instance
				= [&]() -> WideOut {
				// this is anonymous lambda called only once
				return { code_page };
			}(); // call immediately
			return single_instance;
		};

		friend const Printer & printer_instance();

	}; // WideOut

	/* this is Printer's    friend*/
	/* this is also WideOut friend*/
	inline const Printer & printer_instance()
	{
		static Printer single_instance
			= [&]() -> Printer 
		{
			// we can do this only because from inside config we do not 
			// use WideOut or Printer
			auto DBJ_MAYBE(is_it_) = console_is_initialized();
			static WideOut & console_engine_ = WideOut::instance();
			return { &console_engine_ };
		}(); // call immediately but only once!
		return single_instance;
	}

	inline std::wstring get_font_name()
	{
		HANDLE handle_ = printer_instance().cons().handle();
		CONSOLE_FONT_INFOEX cfi = get_current_font_(handle_);
		return { cfi.FaceName };
	}

#pragma endregion 

	inline void paint(const painter_command & cmd_) {
		painter_commander().execute(cmd_);
	}

	namespace config {

		using namespace ::std;
		using namespace ::std::string_view_literals;

		/*
		TODO: usable interface for users to define this
		*/
		inline const bool & instance()
		{
			static auto configure_once_ = []() -> bool
			{
				try {
					::dbj::console::set_font(
						::dbj::console::default_font
					);
					::dbj::core::trace(L"\nConsole font set to: %s\n", ::dbj::console::default_font);

					// and now the really crazy and important measure 
					// for Windows console
					::system("@chcp 65001");
					::dbj::core::trace(L"\nConsole chcp 65001 done\n");

				}
				catch (...) {
					// can happen before main()
					// and user can have no terminators set up
					// so ...
					std::string message_ 
						= ::dbj::win32::get_last_error_message(
							"dbj console configuration has failed"sv
						);
					::dbj::core::trace(L"\nERROR %s", message_.data());
					// throw dbj::exception(message_);
#pragma warning(push)
#pragma warning(disable: 4127 )
					DBJ_VERIFY(false);
#pragma warning(pop)
				}
				//
				return true;
			}();
			return configure_once_;
		} // instance()

		// inline const bool & single_start = instance();

	} // config

} // dbj::console

namespace dbj {
	inline bool console_is_initialized() {
		static bool is_it_ = ::dbj::console::config::instance();
		return is_it_;
	}
}

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
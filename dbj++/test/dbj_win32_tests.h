#pragma once

DBJ_TEST_SPACE_OPEN(dbj_win32)

using namespace ::std;
using namespace ::std::string_view_literals;

#ifdef DBJ_GEO_INFO
DBJ_TEST_UNIT(_GetGeoInfoEx_)
{
	using namespace std::literals;

	constexpr auto us_ = L"US"sv;
	constexpr auto rs_ = L"RS"sv;

	auto DBJ_MAYBE(us_data) = dbj::win32::geo_info((PWSTR)us_.data());
	auto DBJ_MAYBE(rs_data) = dbj::win32::geo_info((PWSTR)rs_.data());

	dbj::console::print(us_data);
	dbj::console::print(rs_data);
}
#endif // DBJ_GEO_INFO	
	// using namespace  dbj::win::con;
	// typedef typename dbj::console::painter_command CMD;
	using namespace  dbj::win32::sysinfo;

	DBJ_TEST_UNIT(dbjvarioustests) {
			DBJ_TEST_ATOM(user_name());
			DBJ_TEST_ATOM(system_directory());
			DBJ_TEST_ATOM(windows_directory());
	};
	/*
	inspired by
	https://support.microsoft.com/en-us/help/124103/how-to-obtain-a-console-window-handle-hwnd
	*/
	extern "C" inline HWND get_console_hwnd(void)
	{
		constexpr unsigned MY_BUFSIZE = 1024; // Buffer size for console window titles.
		HWND hwndFound{};         // This is what is returned to the caller.
		char pszNewWindowTitle[MY_BUFSIZE]{0}; // Contains fabricated WindowTitle.
		char pszOldWindowTitle[MY_BUFSIZE]{0}; // Contains original WindowTitle.

		// Fetch current window title.
		::GetConsoleTitleA(pszOldWindowTitle, MY_BUFSIZE);
		// Format a "unique" NewWindowTitle.
		// as per advice on WARNING C28159 
		::wsprintfA(pszNewWindowTitle, "%d/%d", (int)GetTickCount64(), (int)GetCurrentProcessId());
		// Change current window title.
		::SetConsoleTitleA(pszNewWindowTitle);
		// Ensure window title has been updated.
		::Sleep(40);
		// Look for NewWindowTitle.
		hwndFound = ::FindWindowA(NULL, pszNewWindowTitle);
		// Restore original window title.
		::SetConsoleTitleA(pszOldWindowTitle);

		return(hwndFound);
	}

DBJ_TEST_SPACE_CLOSE



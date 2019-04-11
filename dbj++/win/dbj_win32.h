#pragma once
//
#include <system_error>
#include "dbj_win_inc.h"
#include "../util/dbj_defval.h"
//
#include "../core/dbj++core.h"

namespace dbj {
	namespace win32 {

		inline bool is_windows_10()
		{
			if (!IsWindows10OrGreater())
			{
				::dbj::core::trace(L"\n\nYou need at least Windows 10 %s\n\n", L"Version Not Supported");
				return false;
			}
			return true;
		}

		inline int last_win32_error() noexcept
		{
			struct last final
			{
				last() noexcept : error_(::GetLastError()) {}
				~last() { ::SetLastError(0); }
				int operator() () const noexcept { return error_; }
			private:
				mutable int error_{};
			};

			int last_error_ = (last{})();
			return last_error_;
		};

// return instance of std::system_error
// which for MSVC STL delivers win32 last error message
// by calling what() on it
		inline auto system_error_instance()
			->  std::system_error
		{
			using namespace std;
		return system_error(error_code(last_win32_error(), system_category()));
		}

		// Returns the last Win32 error message
		// with optional prefix
		inline ::dbj::buf::yanb 
		get_last_error_message(
			std::string_view prompt = std::string_view{}
		)
		{
			//Get the error 
			auto syserr = system_error_instance();
			const char * sys_err_msg = syserr.what();

			if (!prompt.empty()) 
			{
				//use std::string only and if needed
				std::string str_(prompt); str_.append(sys_err_msg);
				return { str_.c_str() };
			}

			return { sys_err_msg };
		}

		// no pointer please, use ""sv literals instead
		// template<typename C> inline auto get_last_error_message( const C * const prompt) = delete;

		namespace sysinfo 
		{
			using STRING = std::wstring;

			namespace inner {
				
				inline DWORD INFO_BUFFER_SIZE = 1024;

				static STRING  infoBuf(INFO_BUFFER_SIZE, (wchar_t)0);
				//
				template<class F, class... Pack>
				constexpr inline auto
					call
					(F&& fun, Pack&&... args) {
					infoBuf.clear();
					infoBuf.resize(INFO_BUFFER_SIZE);
					DBJ_VERIFY(0 != std::invoke(fun, (args)...));
					return (infoBuf);
				}
			}
			// 
			inline STRING computer_name() {
				static STRING computer_name_ =
					inner::call(GetComputerName, inner::infoBuf.data(), &inner::INFO_BUFFER_SIZE);
				return computer_name_;
			}

			inline STRING user_name() {
				static STRING user_name_ =
					inner::call(GetUserName, inner::infoBuf.data(), &inner::INFO_BUFFER_SIZE);
				return user_name_;
			}

			inline STRING system_directory() {
				static STRING system_directory_ =
					inner::call(GetSystemDirectory, inner::infoBuf.data(), inner::INFO_BUFFER_SIZE);
				return system_directory_;
			}

			// return e.g. L"C:\windows\"
			inline STRING windows_directory() {
				static STRING windows_directory_ =
					inner::call(GetWindowsDirectory, inner::infoBuf.data(), inner::INFO_BUFFER_SIZE);
				return windows_directory_;
			}

			// DBJ: what happens if there is no drive C: ?
			// we get the system drive letter
			// the first 3 chars that is, e.g. L"C:\\"
			inline const STRING windows_drive()
			{
				// it is unlikely system drive letter 
				// will change between calls
				static STRING result = windows_directory().substr(0, 3);
				_ASSERTE(result.size() == 3);
				return result;
			}

		} // sysinfo

#ifdef DBJ_GEO_INFO
/* 
geo info api using mechanism 
location two letter codes --> https://www.iso.org/obp/ui/#search/code/
see https://docs.microsoft.com/en-us/windows/desktop/api/winnls/ne-winnls-sysgeotype
for SYSGEOTYPE
PWSTR is WCHAR *, thus for us it is wchar_t *
*/

using geo_info_map_type = std::map<const wchar_t *, std::wstring >;

inline auto geo_info = [](PWSTR location) {

	auto info_query = [](PWSTR location, SYSGEOTYPE query)
		-> std::wstring
	{
		auto use_geo_info = [&](PWSTR geoData, int geoDataCount) {
			return ::GetGeoInfoEx(
				(PWSTR)location,
				(GEOTYPE)query,
				(PWSTR)geoData,
				(int)geoDataCount
			);
		};

		int size = use_geo_info(NULL, 0);
		_ASSERTE(size > 0);

		std::wstring geoData(size, 0);
		int rezult = use_geo_info(geoData.data(), (int)geoData.size());

		if (0 == rezult) {
			const auto err = dbj::win32::last_error();
			if (err == ERROR_INSUFFICIENT_BUFFER)
				return { L"The supplied buffer size was not large enough, or it was incorrectly set to NULL." };
			if (err == ERROR_INVALID_PARAMETER)
				return { L"A parameter value was not valid." };
			if (err == ERROR_INVALID_FLAGS)
				return { L"The values supplied for flags were not valid" };
		}
		_ASSERTE(rezult != 0);
		return geoData;
	};

	geo_info_map_type geo_info_map{};

	geo_info_map[L"GEO_LATITUDE"] = info_query(location, GEO_LATITUDE);
	geo_info_map[L"GEO_LONGITUDE"] = (info_query(location, GEO_LONGITUDE));
	geo_info_map[L"GEO_ISO2"] = (info_query(location, GEO_ISO2));
	geo_info_map[L"GEO_ISO3"] = (info_query(location, GEO_ISO3));
	geo_info_map[L"GEO_FRIENDLYNAME"] = (info_query(location, GEO_FRIENDLYNAME));
	geo_info_map[L"GEO_OFFICIALNAME"] = (info_query(location, GEO_OFFICIALNAME));
	geo_info_map[L"GEO_ISO_UN_NUMBER"] = (info_query(location, GEO_ISO_UN_NUMBER));
	geo_info_map[L"GEO_PARENT"] = (info_query(location, GEO_PARENT));
	geo_info_map[L"GEO_DIALINGCODE"] = (info_query(location, GEO_DIALINGCODE));
	geo_info_map[L"GEO_CURRENCYCODE"] = (info_query(location, GEO_CURRENCYCODE));
	geo_info_map[L"GEO_CURRENCYSYMBOL"] = (info_query(location, GEO_CURRENCYSYMBOL));
	geo_info_map[L"GEO_NAME"] = (info_query(location, GEO_NAME));

	return geo_info_map;

	}; // geo_info
#endif // DBJ_GEO_INFO

/*
Bellow is WIN32 API, adjusted for standard C++ use
dbj::win32::string_compare()
locale sensitive so to be used for "UI string comparisons",
ui sorting, and a such
retval is standard -1,0,1 triplet
*/
inline int string_compare(LPCTSTR str1, LPCTSTR str2, unsigned char ignore_case)
{
	int rez = CompareStringEx(
		LOCALE_NAME_USER_DEFAULT,
		ignore_case == 1 ? LINGUISTIC_IGNORECASE : NORM_LINGUISTIC_CASING,
		str1,
		-1,
		str2,
		-1,
		NULL, NULL, 0
	);
	switch (rez) {
	case CSTR_LESS_THAN: rez = -1; break;
	case CSTR_EQUAL: rez = 0; break;
	case CSTR_GREATER_THAN: rez = 1; break;
	}
	return rez;
}

/*
https://stackoverflow.com/a/54491532/10870835
*/
template <typename CHR, typename win32_api>
inline std::basic_string<CHR>
string_from_win32_call(win32_api win32_call, unsigned initialSize = 0U)
{
	std::basic_string<CHR> result((initialSize == 0 ? MAX_PATH : initialSize), 0);
	for (;;)
	{
		auto length = win32_call(&result[0], (int)result.length());
		if (length == 0)
		{
			return std::basic_string<CHR>();
		}

		if (length < result.length() - 1)
		{
			result.resize(length);
			result.shrink_to_fit();
			return result;
		}

		const auto rl_ = result.length();
		result.resize(rl_ + rl_);
	}
}

inline char * basename(
	char * full_path,
	bool remove_suffix = true,
	char delimiter = '\\')
{
	assert(full_path && (full_path[0] != char(0)));
	char * base_ = strrchr(full_path, delimiter);
	base_ = (base_ == NULL ? full_path : base_);
	if (remove_suffix == false) return base_;
	char * dot_pos = strchr(base_, '.');
	if (dot_pos) *dot_pos = char(0);
	return base_ + 1;
}

inline dbj::buf::yanb module_basename(HINSTANCE h_instance = NULL ) {

	std::string module_path
		= string_from_win32_call<char>([h_instance](char* buffer, int size)
	{
		return GetModuleFileNameA(h_instance, buffer, size);
	});

	_ASSERTE(module_path.empty() == false);

	return { basename(module_path.data()) };
}

} // win32
} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
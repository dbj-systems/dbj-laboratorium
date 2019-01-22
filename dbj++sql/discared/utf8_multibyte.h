#pragma once
/*
It's recommended to not use wchar_t because it's not portable.
Use narrow char UTF-8 encoded strings instead. SQLite assumes all narrow
char strings are UTF-8, even for file names, even on Windows.
Using this approach you can get the original code to work just by saving
the file as UTF-8 without BOM. This will make the string

 char q[] = "SELECT * FROM 'mydb' WHERE 'something' LIKE '%ĂÎȘȚÂ%'";

UTF-8 encoded on major compilers (including MSVC and GCC).
Thus. For WIN32 we simply do not use wchar_t
Bellow are just the two helpers one can use to pass
wide char strings in and out

https://alfps.wordpress.com/2011/11/22/unicode-part-1-windows-console-io-approaches/	*/
#ifdef _MSC_VER
namespace {
	// Convert a wide Unicode string to an UTF8 string
	std::string wide_to_multi(std::wstring_view wstr)
	{
		if (wstr.empty()) return {};
		int size_needed = ::WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		::WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
		return strTo;
	}

	// Convert an UTF8 string to a wide Unicode String
	std::wstring multi_to_wide(std::string_view str)
	{
		if (str.empty()) return {};
		int size_needed = ::MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		::MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
		return wstrTo;
	}
}
#endif
/*
Copyright 2018 by DBJ.Systems Ltd

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "pch.h"

#include "../dbj++log/src/dbj++log/dbj++log.hpp"

using namespace std;
using namespace std::string_view_literals;


	// return font name and true/false 
	// depending on is it true type or not
static auto console_font_info()
{
	HANDLE output_ = GetStdHandle(STD_OUTPUT_HANDLE);
	DBJ_VERIFY(output_ != INVALID_HANDLE_VALUE);
	CONSOLE_FONT_INFOEX info{ sizeof(CONSOLE_FONT_INFOEX) };
	DBJ_VERIFY(0 != GetCurrentConsoleFontEx(output_, false, &info));

	return make_pair(
		wstring( info.FaceName ),
		(info.FontFamily & TMPF_TRUETYPE) == TMPF_TRUETYPE
    );
}


int main()
{

	LOGF(WARNING, "Printf-style syntax is also %s", "available");
	/*
	 To change the code page of the console from your program
	 https://docs.microsoft.com/en-us/windows/console/setconsoleoutputcp
	 */
	DBJ_VERIFY( 0 != SetConsoleOutputCP(CP_UTF8));

	SetConsoleTitleW(THIS_APP_TITLE);
	// Win10 PRO 
	::system("@cls");
	::system("@echo off");
	::system("@echo.");
	::system("@echo Set up your console to display UTF-8 text");
	::system("@echo.");
	::system("@echo (c) (" __DATE__ ") by dbj@dbj.org version:[" __DATE__ "][" __TIME__ "]");
	::system("@echo.");
	//::system("@echo This app now will do: chcp 65001");
	::system("@chcp 65001");
	// ::system("@chcp");
	::system("@echo.");
	::system("@echo If specimen bellow is not readable, please change the console");
	::system("@echo font to the one that will make the specimen fully readable");
	::system("@echo Repeat untill you guessed the right font");
	// start this app
	//
	// кошка 日本
	constexpr auto specimen =
		L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd";

	DBJ_VERIFY(_setmode(_fileno(stdout), _O_U8TEXT) != -1);
	// should display: кошка 日本
	::wprintf(L"\n\n\tspecimen:\t%s\n\n", specimen);

	auto [font_name, font_is_tt_ ] = console_font_info();
	const wchar_t * not_ = (font_is_tt_ ? L"" : L"NOT");
	::wprintf(L"\n\n\tCurrent font name is '%s', this is %s true type font.\n\n", 
		font_name.c_str(), not_);

	::system("@echo.");
	::system("@echo If you see the specimen fully and clearly, you have found the right font");
	::system("@echo Each time you need to display UTF-8 text properly in the console on this desktop");
	::system("@echo You will have to use this font and execute 'chcp 65001', before that.");
	::system("@echo.");
	::system("@pause");
	return 1;
}

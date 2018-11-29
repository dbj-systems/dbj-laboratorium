#include "pch.h"
using namespace std;
using namespace std::string_view_literals;

int main()
{
	// Win10 PRO 
	::system("@cls");
	::system("@echo off");
	::system("@echo.");
	::system("@echo Set up your console to display UTF-8 text");
	::system("@echo.");
	::system("@echo (c) (" __DATE__ ") by dbj@dbj.org version:[" __DATE__ "][" __TIME__ "]");
	::system("@echo.");
	::system("@echo This app now will do: chcp 65001");
	::system("@echo.");
	::system("@chcp 65001");
	::system("@echo.");
	::system("@echo If specimen bellow is not readable, please change the console");
	::system("@echo font to the one that will make the specimen fully readable");
	::system("@echo Please do this change now");
	::system("@echo Repeat untill you guessed the right font");
	// start this app
	//
	// кошка 日本
	constexpr auto specimen =
		L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd";

	// int result = _setmode(_fileno(stdout), _O_U16TEXT);
	int result = _setmode(_fileno(stdout), _O_U8TEXT);
	// int result = _setmode(_fileno(stdout), _O_WTEXT);
	// int result = _setmode(_fileno(stdout), _O_TEXT);
	// int result = _setmode(_fileno(stdout), _O_BINARY);
	if (result == -1) {
		perror("Cannot set mode");
		exit(EXIT_FAILURE);
	}
	// should display: кошка 日本
	// no crash on any mode
	::wprintf(L"\n\n\tspecimen:\t%s\n\n", specimen);

	// for any mode the following crashes 
	// the UCRT (aka Universal CRT)
	// crashes on
	// _O_U16TEXT	-- crash
	// _O_U8TEXT	-- crash
	// _O_WTEXT		-- crash
	// _O_TEXT		-- no crash but hardly any output
	// _O_BINARY	-- no crash but hardly any output
	//
	// ::printf("\nprintf()\n\tspecimen: %S\n", specimen);

	::system("@echo.");
	::system("@echo If you see the specimen fully and clearly, you have found the right font");
	::system("@echo Each time you need to display UTF-8 text properly in the console on this desktop");
	::system("@echo You will have to use this font and execute 'chcp 65001', before.");
	::system("@echo.");
	::system("@pause");
	return 1;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

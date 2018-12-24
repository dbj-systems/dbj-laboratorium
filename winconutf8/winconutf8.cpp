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

static void unicode_specimen() 
{
	/*
	Microsoft Windows [Version 10.0.17134.471], i5, 8GB RAM
	This works with font: NsimSun
	*/

	// кошка 日本
	constexpr auto specimen =
		u8"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd";

	// this is non-standard UCRT printf extension
	::printf("\nusing %%hS -- %hS", specimen);
	::printf("\nusing %%TS -- %TS", specimen);
}

int main()
{
	/*
	I am on: Microsoft Windows [Version 10.0.17134.471]
	i5, 8GB RAM

	defined in winnls.h:
	#define CP_UTF8 65001 // UTF-8 translation
	*/
	_ASSERT_AND_INVOKE_WATSON( 0 != ::SetConsoleOutputCP(CP_UTF8));

	::system("@cls");
	::system("@echo off");
	::system("@ver");
	::system("@echo.");
	::system("@chcp"); // prints: Active Code Page 850
	::system("@echo.");
	::system("@pause");

	return  EXIT_SUCCESS;
}

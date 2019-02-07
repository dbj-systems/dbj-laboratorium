#pragma once
#include "pch.h"


// DBJ_TESTING_ONAIR

namespace dbj::samples {

	/* */
	DBJ_TEST_UNIT(string_literals_with_universal_character_names) {

		using dbj::console::print;

		/*
		https://docs.microsobase_type.com/en-us/cpp/cpp/string-and-character-literals-cpp
		*/
		// ASCII smiling face  
		const char*     s1 = ":-)";
		// UTF-16 (on Windows) encoded WINKING FACE (U+1F609)  
		const wchar_t*  s2 = L"😉 = \U0001F609 is ;-)";
		// UTF-8  encoded SMILING FACE WITH HALO (U+1F607)  
		const char*     s3 = u8"😇 = \U0001F607 is O:-)";
		// UTF-16 encoded SMILING FACE WITH OPEN MOUTH (U+1F603)  
		const char16_t* s4 = u"😃 = \U0001F603 is :-D";
		// UTF-32 encoded SMILING FACE WITH SUNGLASSES (U+1F60E)  
		const char32_t* s5 = U"😎 = \U0001F60E is B-)";

		print("ASCII smiling face ")(s1);
		print("UTF-16 (on Windows) encoded WINKING FACE (U+1F609)")(s2);
		print("UTF-8  encoded SMILING FACE WITH HALO (U+1F607)")(s3);
		print("UTF-16 encoded SMILING FACE WITH OPEN MOUTH (U+1F603)")(s4);
		print("UTF-32 encoded SMILING FACE WITH SUNGLASSES (U+1F60E)")(s5);
	}
} // dbj::samples

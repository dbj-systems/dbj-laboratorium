#pragma once
#include "pch.h"


// DBJ_TESTING_ONAIR

namespace dbj::samples {

	auto out = [&]( const auto & s_) 
	{
		using namespace std;
		using FT = dbj::tt::to_base_t< decltype(s_) >;
		auto type_name_ =  DBJ_TYPENAME(FT);

		if constexpr (dbj::tt::same_types<FT, char>) {
			cout << "\n" << s_ << "\n";
		} else
			if constexpr (dbj::tt::same_types<FT, wchar_t>) {
				wcout << "\n" << s_ << "\n";
			}
			else if constexpr (dbj::tt::same_types<FT, char16_t>) {
				wcout << "\n" << s_ << "\n";
			}
			else if constexpr (dbj::tt::same_types<FT, char32_t>) {
				wcout << "\n" << s_ << "\n";
			}
			else {
				cout << "\nCan not print type:" << type_name_ << "\n";
			}

		return out ;
	};
	/* */
	DBJ_TEST_UNIT(string_literals_with_universal_character_names) {

		auto sync_state_previous = std::ios::sync_with_stdio(true);
		/*
		https://docs.microsoft.com/en-us/cpp/cpp/string-and-character-literals-cpp
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

		out("ASCII smiling face ")(s1);
		out("UTF-16 (on Windows) encoded WINKING FACE (U+1F609)")(s2);
		out("UTF-8  encoded SMILING FACE WITH HALO (U+1F607)")(s3);
		out("UTF-16 encoded SMILING FACE WITH OPEN MOUTH (U+1F603)")(s4);
		out("UTF-32 encoded SMILING FACE WITH SUNGLASSES (U+1F60E)")(s5);

		std::ios::sync_with_stdio(sync_state_previous);
	}
} // dbj::samples

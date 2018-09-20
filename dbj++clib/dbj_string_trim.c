
/*
Copyright 2018 by dbj@dbj.org
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

#include "dbjclib.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

	typedef unsigned int size_t;
	static const int EOS = (int)'\0';
	/*
	static const char * digits[] = { "0123456789" };
	static const char * uppercase_letters[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
	static const char * lowercase_letters[] = { "abcdefghijklmnopqrstuvwxyz" };
	*/

	inline bool dbj_isalnum(unsigned char c)
	{
		return 
			(c >= '0' && c <= '9') &&
			(c >= 'A' && c <= 'Z') &&
			(c >= 'a' && c <= 'z');
	}

	inline bool dbj_isspace(unsigned char c)
	{
		return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v';
		// add whatever else you consider space
	}

	inline void string_trim_front_back_finder(char ** begin_, char ** end_)
	{
		assert(*end_);
		assert(*begin_);

		if (*begin_ == *end_) return;

		while (dbj_isspace(**begin_)) {
			(*begin_)++;
		};

		// all spaces case
		if (*begin_ == *end_) return;

		// if EOS (aka 0)
		// it is not a space (aka 32)
		// thus we have to move it "left" 1 first
		// before the first check
		if (**end_ == EOS)
			;
			(*end_)--;
		while (dbj_isspace(**end_)) {
			(*end_)--;
		};
		// move it back
		// beyond the last non space char
		(*end_)++;
	}

	// must be zero limited string
	// that is with EOS ('\0') a the very end
	void dbj_string_trim(const char * text, char ** p1, char ** p2)
	{
		assert(text);
		size_t text_len = strlen(text);
		*p1 = (char *)& text[0];
		*p2 = (char *)& text[text_len];
		string_trim_front_back_finder(p1, p2);
	}

// C++17 usage
#if (1==0)
#include <string>
#include <string_view>

#if ! _HAS_CXX17
#error C++17 required
#endif

	// must be limited string
	// that is with EOS ('\0') a the end
	inline std::string trimmer(::std::string_view text)
	{
		char * p1 = 0;
		char * p2 = 0;
		string_trim(text.data(), &p1, &p2);
		return { p1, p2 };
	}

int main()
{
	using namespace ::std::string_view_literals;

	auto target = "LINE O FF    TE   XT"sv;
	std::string_view text[]{
		{ "   LINE O FF    TE   XT    "sv },
		{ "   LINE O FF    TE   XT"sv },
		{ "LINE O FF    TE   XT"sv },
		{ "     "sv }
	};

	_ASSERTE(target == trimmer(text[0]));
	_ASSERTE(target == trimmer(text[1]));
	_ASSERTE(target == trimmer(text[2]));
	// on trim, spaces are collapsing 
	// to empty string
	_ASSERTE("" == trimmer(text[3]));
}
#endif

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

	// static const int		EOS = (int)'\0';
	/*
	static const char * digits[] = { "0123456789" };
	static const char * uppercase_letters[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
	static const char * lowercase_letters[] = { "abcdefghijklmnopqrstuvwxyz" };
	*/

	// typedef bool(*dbj_string_trim_policy)(unsigned char);

	bool dbj_seek_alnum(uchar_t c)
	{
		if (c >= '0' && c <= '9') return false;
		if (c >= 'A' && c <= 'Z') return false;
		if (c >= 'a' && c <= 'z') return false;
		// c is not alnum so move the pointer
		// by returning true
		return true;
	}

	// c is space so move the pointer by returning true
	bool dbj_is_space(uchar_t c)
	{
		return c == ' ';
	}

	// any kind of white space ? 
	// yes: move the pointer by returning true
	bool dbj_is_white_space(uchar_t c)
	{
		return c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f' ;
		// add whatever else you consider white space
	}

/*
ask the driver to move the pointer
if c is whitespace, space or eos
not using locale but should be prety resilient to local chars
since whitespace, space and eos are not locale specific
*/
	bool dbj_move_if_not_alnum(uchar_t c) {
		// positioned on EOS char
		// thus move the pointer
		if ((int)c == 0) return true;
		//
		if (dbj_is_white_space(c)) return true;
		//
		if (dbj_is_space(c)) return true;
		// ask the driver to stop the movement
		// since c is not any of the above
		return false;
	}

	/*
	gloobal 'holder' of the current policy function
	defualt policy is char == ' ' 
	*/
	dbj_string_trim_policy current_dbj_string_trim_policy = dbj_is_space ;

	/*
	internal front/back driver
	this is the policy user
	*/
	inline void string_trim_front_back_driver(char ** begin_, char ** end_)
	{
		// empty string case
		if (*begin_ == *end_) return;

		while (current_dbj_string_trim_policy(**begin_)) {
			(*begin_)++;
			// depending on the policy
			// trim result has collapsed into empty string
			if (*begin_ == *end_) return;
		};

		// the right trim
		while (current_dbj_string_trim_policy(**end_)) {
			(*end_)--;
		};
	}

// if *back_ is NULL then text_
// must be zero limited string
// that is with EOS ('\0') a the very end
// REMEMBER:
// to conform to the STL meaning of "end"
// user has to move the back_ result
// one to the right
	void dbj_string_trim( const char * text_,	char ** front_, char ** back_	)
	{
		assert(text_);
		assert(current_dbj_string_trim_policy);
		
		// do not assume front points to slot [0]
		// of the input string
		if ( *front_ == NULL )
		*front_ = (char *)& text_[0];
		
		if (*back_ == NULL) {
			size_t text_len = strlen(text_);
			// notice how back_ is not the C++ end_
			*back_ = (char *)& text_[text_len-1];
		} 

		assert(*front_);
		assert(*back_);

		// return on empty text
		if (*front_ == *back_ ) return;
		// move the pointers according to policy
		string_trim_front_back_driver(front_, back_);
	}

// EOF
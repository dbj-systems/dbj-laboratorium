#include "../dbjclib.h"

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
	// static const int		EOS = (int)'\0';
	/*
	static const char * digits[] = { "0123456789" };
	static const char * uppercase_letters[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
	static const char * lowercase_letters[] = { "abcdefghijklmnopqrstuvwxyz" };
	*/

	// typedef bool(*dbj_string_trim_policy)(unsigned char);


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
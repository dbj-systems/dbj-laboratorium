/*
Copyright 2017 by dbj@dbj.org

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
/*
  Concepts and the Design
  ***********************

The key abstraction is char array with front and back pointers.
char array can contain any unsigned char in the range 0 - 127
for the oridnals and  meaning of particular chars see the table here:
https://en.cppreference.com/w/cpp/string/byte/isalnum

Example of an unsigned char array is thus:

Front                                                                 Back
  |                                                                     |
  V                                                                     V
+---+---+---+---+---+---+---+---+---+---+---+---+----+---+----+---+---+---+
|   |\f | A | b | r | a |\t | K | a |\n | D | a | b  | r |  a |\v |   | 0 |
+---+---+---+---+---+---+---+---+---+---+---+---+----+---+----+---+---+---+
  0   1   .   .   .                                                     17

By "trimming" we mean moving the front to the right and
back to the left. front_back_driver function, moves this pointers by using
the current policy function.

What is the trimming policy function?

The policy that is used to trim is actually what delivers
a logic that drives a front/back pointers, by returning 
true or false.

Pointer is moved on true returned from a policy function,
front to the right or back pointer to the left
policy function argument is the char value to which the current
back/front pointer is pointing.

IMPORTANT: this is powerfull trimming suite. The trimming can be 
started with arbitrary front and end pointers, or user defined Back pointer that is not the result 
of simple strlen. 

When  defining the policy function be aware that Front and Back can 
point to any position before triming starts. Not just first char for the Front 
and  strlen(text) - 1 for the Back.

If your policy does not care for that the results will be very likely
not what is expected.

Normal Example

First, let's define the policy function logic to drive the pointer for 
any char that is not alpha or numeric. True return moves the back/front

bool move_if_not_alphanum ( uchar_t current_char ) {  return ! isalnum( (char)current_char ); }

Now we assign this as the current policy by using the global function pointer:

dbj::clib::current_dbj_string_trim_policy = move_if_not_alphanum ;

Then we start the trimming.

char text[]{" \fA bra\tKa\nDabra\v " } ; 
char * front = text[0] ;
char * back  = text[ sizeof(text)] ;
dbj_string_trim( text, &front, &back ) ;

The situation is now: 

        Front                                             Back
          |                                                 |
          V                                                 V
+---+---+---+---+---+---+---+---+---+---+---+---+----+---+----+---+---+---+
|   |\f | A | b | r | a |\t | K | a |\n | D | a | b  | r |  a |\v |   | 0 |
+---+---+---+---+---+---+---+---+---+---+---+---+----+---+----+---+---+---+

To make std string from this we can do:

 return std::string{ front, back+1};

And here is one important caveat: C++ concept of "end" is not the same as "back".
In C++ "end" is one beyond the last. "Back" is the last one in this case.

Warning
*******

As already pointed out. If user trims non zero limited string, the user is responsible 
for intepreting the output. That is user defined logic requires care when interpreting 
the result. For example this input

 Front                                                                Back
  |                                                                     |
  V                                                                     V
+---+---+---+---+---+---+---+---+---+---+---+---+----+---+----+---+---+---+
|   |   |   |   |   |   |   |   |   |   |   |   |    |   |    |   |   | 0 |
+---+---+---+---+---+---+---+---+---+---+---+---+----+---+----+---+---+---+

Can result in this situation after the trimming

                                                           Front == Back
                                                                    |
                                                                    V
+---+---+---+---+---+---+---+---+---+---+---+---+----+---+----+---+---+---+
|   |   |   |   |   |   |   |   |   |   |   |   |    |   |    |   |   | 0 |
+---+---+---+---+---+---+---+---+---+---+---+---+----+---+----+---+---+---+

if then user creates std string, as advised by moving 
the back pointer 1 to the right

return std::string( front, back + 1) ;

The resulting string will be the equivalent of (in C++) : char [1]{0} ;


Singularities
*************

Empty string trimming does nothing. But beware. See above.

NOTE: 

Naive (and basic) string trimming, concept is  to "trim" the string by
actually shortening it, by inserting 0 aka "end of string" where required
Which in C/C++ is better not to be done. This is because of R/O memory issue. 
Example:

const char * ro = "READONLY";
char * bang_ = (char *) & ro[0] ;
 // runtime write access violation happens here 
bang_[0] = '!';

*/



#pragma once
#include <stdbool.h>
#if defined( __clang__ ) && ! defined( __cplusplus )

# if !defined(__STDC_VERSION__) ||  (__STDC_VERSION__ < 199901L)
/* Your compiler is not conforming to C99, since
   this requires the macro __STDC_VERSION__ to be set to the
   indicated value (or larger).

   NOTE: For C11, __STDC_VERSION__ == 201112L
*/
#define DBJ_EVAL_REMOVE(expr) typedef char __static_assert_t[(expr) != 0]
#define DBJ_REMOVE(expr) typedef char __static_assert_t[sizeof(expr) != 0]
#else
#define DBJ_EVAL_REMOVE(expr) typedef char __static_assert_t[(expr) != 0]
#define DBJ_REMOVE(expr) typedef char __static_assert_t[sizeof(expr) != 0]
# endif
/*
#	if ! defined(_WCHAR_T_DEFINED)
#error	Need _WCHAR_T_DEFINED
#	endif
*/

#	if ! defined(_MSC_EXTENSIONS)
#error Need MSC EXTENSIONS DEFINED
#	endif


#if !defined( _WIN32 ) && !defined(_WIN64)
#error Need _WIN32 or _WIN64
#endif

#endif
/*
Note: while inside c++ this is all in the dbj::clib namespace
*/
#ifdef __cplusplus
namespace dbj::clib {

	extern "C" {
#endif
#pragma region string triming with policies
typedef unsigned char	uchar_t;
typedef unsigned int	size_t;


typedef bool(*dbj_string_trim_policy)(unsigned char);

/* 
policies present in the library 
*/
/* return true if c is not alnum */
bool dbj_seek_alnum(uchar_t c);
bool dbj_is_space(uchar_t c);
bool dbj_is_white_space(uchar_t c);
/*
ask the driver to move the pointer 
if c is whitespace, space or eos
not using locale but should be prety resilient to local chars
since whitespace, space and eos are not locale specific
*/
bool dbj_move_if_not_alnum(uchar_t c);
/* 
dbj_is_space is default policy, it equates to  char == ' ', test  
users can provide their own drivers
by assigning to the global bellow
Note: while inside c++ this is all in the dbj::clib namespace
*/
extern dbj_string_trim_policy current_dbj_string_trim_policy ;

// if *back_ is NULL then text_
// must be zero limited string
// that is with EOS ('\0') a the very end
//
// REMEMBER:
// to conform to the C++ meaning of "end"
// user has to move the back_
// one to the right, *before* using it in STD space
// for example.
//
// std::string rezult( front_, back_ + 1 ) ; 
//
void dbj_string_trim( const char * text_, char ** front_, char ** back_ );
#pragma endregion 

#pragma region location descriptor
typedef struct location_descriptor location_descriptor;

#define location_descriptor_file_name_size 1024U

typedef struct location_descriptor {
	unsigned int cache_index;
	unsigned int line;
	char file[location_descriptor_file_name_size];
} location_descriptor;

typedef struct LOCATION LOCATION;

typedef struct LOCATION {

	location_descriptor * 
		(*create)
		(const int line_, const char * file_);

	location_descriptor *
		(*release)
		(location_descriptor **);

} LOCATION;

extern LOCATION location_;
#pragma endregion 

#ifdef __cplusplus
	} // extern "C"
#pragma region testing string triming with policies
	namespace test {

		using namespace ::std;

		// DBJ proposal :
		//
		// void using namespace ::std;
		//
		// stop using namespace

		// testing data
		inline auto target = "LINE O FF\n\rTE\v\tXT"sv;
		constexpr inline std::string_view text_test_data[]{
			{ "   LINE O FF\n\rTE\v\tXT    "sv },
			{ "   LINE O FF\n\rTE\v\tXT"sv },
			{    "LINE O FF\n\rTE\v\tXT"sv },
			// all spaces singularity
			{ "     "sv },
			// empty string singularity
			{ ""sv }
		};

		// trim the string view buffer
		// return the result in a string
		inline std::string trimmer(
			string_view text,
			// if false send the whole buffer
			bool zero_limited_string = true
		)
		{
			char * front_ = 0, * back_  = 0;

			// if requested set back_ pointer
			// to the end of the buffer position
			if (!zero_limited_string) {
				back_ = (char *)(text.data() + text.size());
			}

			DBJ::clib::dbj_string_trim(	text.data(), &front_, &back_ );

			if ( (! zero_limited_string) && (*front_ == 0))
				return ""s;
			// if not back_ + 1 , c++ will cut of the *back_ 
			return { front_, back_ +1 };
		}

		inline void dbj_string_trim_test() {
			using namespace string_view_literals;
			using namespace string_literals;

			// the classic use case is trimming spaces
			// from zero delimited string literals
			// using default triming policy ( char == ' ')
			current_dbj_string_trim_policy = dbj_is_space;

			auto trim_assert = []( auto required_outcome, size_t test_data_index, bool zero_delimited_strings = true ) {
				_ASSERTE(
					required_outcome == trimmer(text_test_data[test_data_index], zero_delimited_strings)
				);
			};

			// using zero delimited strings
			// results are predictable
			trim_assert(target, 0);
			trim_assert(target, 1);
			trim_assert(target, 2);
			trim_assert(" "s, 3);
			trim_assert(""s, 4);

			// using the whole buffer
			// that is: NON zero limited strings?
			// That is fine, *if* we change the policy
			// as the first current char might be EOS
			// when we start moving the back_ pointer
			// this policy will also provoke moving over anything 
			// that is not alphanum but in a more 
			// locale resilient manner
			current_dbj_string_trim_policy = dbj_move_if_not_alnum;

			trim_assert(target, 0, false);
			trim_assert(target, 1, false);
			trim_assert(target, 2, false);

			// results are not that predictable
			// when trimming char array vs strings 
			// with user defined policies
			// in here for example
			// all spaces input will collapse to 
			// char[1]{0} not char * to ""
			auto r_0 = trimmer(text_test_data[3], false);
			// same is for empty string input
			auto r_1 = trimmer(text_test_data[4], false);

		}
	} // test
#pragma endregion
} // eof namespace dbj::clib 
#endif


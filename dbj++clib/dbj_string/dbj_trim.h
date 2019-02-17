#pragma once

#pragma region string triming with policies

		typedef unsigned char	uchar_t;
		typedef bool(*dbj_string_trim_policy)(unsigned char);
/*
#ifdef _WIN64
		typedef unsigned __int64 size_t;
#else
		typedef unsigned int	size_t;
#endif
*/
		/*
		basic policies present in the library
		*/
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
		extern dbj_string_trim_policy current_dbj_string_trim_policy;

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
		void dbj_string_trim(const char * text_, char ** front_, char ** back_);
#pragma endregion 


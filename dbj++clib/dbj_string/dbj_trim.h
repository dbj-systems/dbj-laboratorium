#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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
	inline bool dbj_seek_alnum(uchar_t c)
	{
		if (c >= '0' && c <= '9') return false;
		if (c >= 'A' && c <= 'Z') return false;
		if (c >= 'a' && c <= 'z') return false;
		// c is not alnum so move the pointer
		// by returning true
		return true;
	}

	// c is space so move the pointer by returning true
	inline bool dbj_is_space(uchar_t c)
	{
		return c == ' ';
	}

	// any kind of white space ? 
	// yes: move the pointer by returning true
	inline bool dbj_is_white_space(uchar_t c)
	{
		return c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
		// add whatever else you consider white space
	}

	/*
	make the driver to move the pointer
	if c is whitespace, space or eos
	not using locale but should be prety resilient to local chars
	since whitespace, space and eos are not locale specific
	*/
	inline bool dbj_move_if_not_alnum(uchar_t c) {
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

#ifdef __cplusplus
} // extern "C" 
#endif


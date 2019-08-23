#pragma once

namespace dbj {
	
/*
2018-04-30	dbj@dbj.org	 -- created	basically as a C code 
*/
extern "C" {
	/*
	*Entry:
	*       const char *_string1 = pointer to beginning of the first string
	*       const char *s1e = pointer past end of the first string
	*       const char *_string2 = pointer to beginning of the second string
	*       const char *_end2 = pointer past end of the second string
	*
	*Exit:
	*       Less than 0    = first string less than second string
	*       0              = strings are equal
	*       Greater than 0 = first string greater than second string
	*/
	inline int __cdecl dbj_ordinal_compareA(
		const char *_string1,
		const char *_end1,
		const char *_string2,
		const char *_end2)
	{
		const int n1 = (int)((_end1 - _string1));
		const int n2 = (int)((_end2 - _string2));

		const int ans = memcmp(_string1, _string2, n1 < n2 ? n1 : n2);
		const int ret = (ans != 0 || n1 == n2 ? ans : n1 < n2 ? -1 : +1);

		return ret;
	}

	inline int __cdecl dbj_ordinal_compareW(
		const wchar_t *_string1,
		const wchar_t *_end1,
		const wchar_t *_string2,
		const wchar_t *_end2)
	{
		const int n1 = (int)((_end1 - _string1));
		const int n2 = (int)((_end2 - _string2));

		const int ans = wmemcmp(_string1, _string2, n1 < n2 ? n1 : n2);
		const int ret = (ans != 0 || n1 == n2 ? ans : n1 < n2 ? -1 : +1);

		return ret;
	}

	/* must free the result */
	inline char * dbj_lowerize_stringA(const char * the_str_) {
		_ASSERTE(the_str_);
		char * dup = _strdup(the_str_);
		_ASSERTE(dup);
		const size_t sze_ = strlen(dup);
		size_t cnt_ = 0;
		do  {
			dup[cnt_] = (char)tolower(dup[cnt_]);
		} while (cnt_++ < sze_);
		return dup;
	}

	/* must free the result */
	inline wchar_t * dbj_lowerize_stringW(const wchar_t * the_str_) {
		_ASSERTE(the_str_);
		wchar_t * dup = _wcsdup(the_str_);
		_ASSERTE(dup);
		const size_t sze_ = wcslen(dup);
		size_t cnt_ = 0;
		 do {
			dup[cnt_] = towlower(dup[cnt_]);
		 } while (cnt_++ < sze_);
		return dup;
	}

	/// <summary>
	/// ordinal comparison of two ascii null terminated strings
	/// </summary>
	inline bool dbj_ordinal_string_compareA(const char * str1, const char * str2, unsigned char ignore_case) {

	  // much faster, does not transform input strings first
		typedef int(*char_trans)(int);

		char_trans transformer;

		if (ignore_case)
			transformer = dbj::str::dbj_tolower;
		else
			transformer = dbj::str::just_copy_char;

		size_t index_ = 0;
		while ( str1[index_] && str2[index_])
		{
			if (transformer(str1[index_]) != transformer(str2[index_]))
				return false;
			++index_;
		}
		return true;
	}

	/// <summary>
	/// ordinal comparions of two unicode null terminated strings
	/// </summary>
	inline bool dbj_ordinal_string_compareW(const wchar_t * str1, const wchar_t * str2, unsigned char ignore_case) {

		typedef wint_t(*char_trans)(wint_t);

		char_trans transformer;

		if (ignore_case)
			transformer = towlower;
		else
			transformer = dbj::str::just_copy_wchar;

		size_t index_ = 0;
		while (str1[index_] && str2[index_])
		{
			if (transformer(str1[index_]) != transformer(str2[index_]))
				return false;
			++index_;
		}
		return true;
	}


} // extern "C"

} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
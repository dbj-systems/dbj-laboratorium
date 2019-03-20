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
	inline int dbj_ordinal_string_compareA(const char * str1, const char * str2, unsigned char ignore_case) {

		if (ignore_case) {
			char * cp1 = dbj_lowerize_stringA(str1);
			char * cp2 = dbj_lowerize_stringA(str2);
			int rez = dbj_ordinal_compareA(
				cp1, cp1 + strlen(cp1), cp2, cp2 + strlen(cp2)
			);
			free(cp1);
			free(cp2);
			return rez;
		}
		else {
			return dbj_ordinal_compareA(
				str1, str1 + strlen(str1), str2, str2 + strlen(str2)
			);
		}
	}

	/// <summary>
	/// ordinal comparions of two unicode null terminated strings
	/// </summary>
	inline int dbj_ordinal_string_compareW(const wchar_t * str1, const wchar_t * str2, unsigned char ignore_case) {

		if (ignore_case) {
			wchar_t * cp1 = dbj_lowerize_stringW(str1);
			wchar_t * cp2 = dbj_lowerize_stringW(str2);
			int rez = dbj_ordinal_compareW(
				cp1, cp1 + wcslen(cp1), cp2, cp2 + wcslen(cp2)
			);
			free(cp1);
			free(cp2);
			return rez;
		}
		else {
			return dbj_ordinal_compareW(
				str1, str1 + wcslen(str1), str2, str2 + wcslen(str2)
			);
		}
	}


} // extern "C"

} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
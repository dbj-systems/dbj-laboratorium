#pragma once

// https://godbolt.org/z/L_uhKR
// this was apparently not ok for MSVC
// but now (2019AUG) it is or it is not?

namespace dbj::str {

#ifdef _MSC_VER

		// MSVC has a lot of problem when overloading template functions that are  also 
		// trying to decipher what is a native arrat reference and what is a pointer
		// following are working at compile time too
		// and none is expecting native arrays

		//  standard pointer versions of strlenand strnlen
		//	note : str(n)len is charr array length - 1 because it does not count the null byte at the end
		//	note : strnlen is a GNU extension and also specified in POSIX(IEEE Std 1003.1 - 2008).
		//	If strnlen is not available for char arrays
		//	use the dbj::str::strnlen replacement.

		inline constexpr size_t strlen(const char str[1])
		{
			return *str ? 1 + strlen(str + 1) : 0;
		}

		inline constexpr size_t strlen(const wchar_t str[1])
		{
			return *str ? 1 + strlen(str + 1) : 0;
		}

		inline constexpr size_t strlen(const char16_t str[1])
		{
			return *str ? 1 + strlen(str + 1) : 0;
		}

		inline constexpr size_t strlen(const char32_t str[1])
		{
			return *str ? 1 + strlen(str + 1) : 0;
		}

	/*
	Inspired by: https://opensource.apple.com/source/bash/bash-80/bash/lib/sh/strnlen.c
	*/
		inline constexpr size_t strnlen(const char* s, size_t maxlen)
		{
			const char* e = {};
			size_t n = {};
			for (e = s, n = 0; *e && n < maxlen; e++, n++) ;
			return n;
		}

		inline constexpr size_t strnlen(const wchar_t * s, size_t maxlen)
		{
			const wchar_t * e = {};
			size_t n = {};
			for (e = s, n = 0; *e && n < maxlen; e++, n++) ;
			return n;
		}

		inline constexpr size_t strnlen(const char16_t * s, size_t maxlen)
		{
			const char16_t* e = {};
			size_t n = {};
			for (e = s, n = 0; *e && n < maxlen; e++, n++) ;
			return n;
		}

		inline constexpr size_t strnlen(const char32_t * s, size_t maxlen)
		{
			const char32_t* e = {};
			size_t n = {};
			for (e = s, n = 0; *e && n < maxlen; e++, n++) ;
			return n;
		}

#endif // _MSVC_VER

#if defined( __clang__ ) || defined( __GNUC__ )



	/*
	(c) 2017, 2018 by dbj.org
	"zero" time modern C++ versions of str(n)len
	this should speed up any modern C++ code ... perhaps quite noticeably
	*/
	/*
	in here we cater for char, wchar_t, char16_t, char32_t
	for details please see https://docs.microsoft.com/en-us/cpp/cpp/char-wchar-t-char16-t-char32-t

	*/
	template<typename T, size_t N>
	constexpr inline size_t strnlen(
		const T(&carr)[N],
		const size_t& maxlen
	)
	{
		static_assert(dbj::is_std_char_v< std::remove_cv_t<T>>,
			"[dbj strnlen] requires only standard chars");

		return std::min(N, maxlen) - 1;
	}

	// stop array of pointers shennanigans
	template<typename T, size_t N>
	inline size_t strnlen(
		const T(*carr)[N], const size_t& maxlen
	) = delete;

	/*
	strlen for C++ native char array reference
	of course this requires zero terminated strings
	*/
	template<typename T, size_t N>
	constexpr inline size_t strlen(T const (&carr)[N])
	{
		static_assert(dbj::is_std_char_v< std::remove_cv_t<T>>,
			"[dbj strlen] requires only standard chars");
		return N - 1;
	}

	// stop pointer to array shennanigans
	template<typename T, size_t N>
	inline size_t strlen(const T(*carr)[N]) = delete;


	template<typename C,
		typename char_type = std::remove_cv_t<C> >
		inline constexpr size_t strlen(const char_type * cp)
	{
		static_assert(dbj::is_std_char_v< char_type >,
			"[dbj strlen] requires only standard chars");

		return   std::basic_string_view<char_type>{cp}.size();
		// return std::char_traits< std::remove_cv_t<C> >::length(cp);
	}
	// this was non MSVC version only
	// now it is not
	template<typename T>
	inline size_t strnlen(T* const* cp, const size_t& maxlen)
	{

		size_t cpl = cp.size(); //  std::char_traits<T>::length(cp);
		return (cpl > maxlen ? maxlen : cpl);
	}
#endif
}

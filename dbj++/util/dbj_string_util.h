#pragma once
/*

UNLESS OTHERWISE STATED THIS CODE IS GOOD ONLY FOR FIRST 127 CHARS
IN ASCI locale unaware situations

*/

#include "../core/dbj_crt.h"
#include "../core/dbj_traits.h"

// #include <type_traits>
#include <locale>
#ifdef DBJ_USE_STD_STREAMS
#include <sstream> // wstringstream
#endif
#include <memory>  // allocator
#include <string>
#include <optional>

namespace dbj::str {

#pragma region low level

	extern "C" {

	inline void reverse(char * str, size_t N)
	{
		size_t start = 0;
		size_t end = N - 1;
		while (start < end)
		{
			std::swap(*(str + start), *(str + end));
			start++;
			end--;
		}
	}

	// Implementation of itoa()  depends on the reverse above
	// note: this is low level, be sure str has enough space
	inline char* itoa(int num, char* str, int base)
	{
		int i = 0;
		bool isNegative = false;

		/* Handle 0 explicitly, otherwise empty string is printed for 0 */
		if (num == 0)
		{
			str[0] = '0';
			str[1] = '\0';
			return str;
		}

		// In standard itoa(), negative numbers are handled only with  
		// base 10. Otherwise numbers are considered unsigned. 
		if (num < 0 && base == 10)
		{
			isNegative = true;
			num = -num;
		}

		// Process individual digits 
		while (num != 0)
		{
			int rem = num % base;
			str[i++] = (char)
				(
				(rem > 9) ? (rem - 10) + 'a' : rem + '0'
					);
			num = num / base;
		}

		// If number is negative, append '-' 
		if (isNegative)
			str[i++] = '-';

		str[i] = '\0'; // Append string terminator 

		// Reverse the string 
		reverse(str, i);

		return str;
	}

		// locale unaware, for char 0 - char 127
		inline bool isalpha(int c)
		{
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
		}

		inline bool iswhitespace(int c)
		{
			// dbj note: what is with \b (aka BELL) escape code?
			return c == '\t' || c == '\r' || c == '\v' || c == '\n' || c == '\f';
		}

		inline bool isspace(int c)
		{
			return c == 32;
		}

		inline bool ispunct(int c)
		{
			static const char *punct = ".;!?...";
			return strchr(punct, c) == NULL ? false : true;
			// can make this shorter
		}

		// locale unaware, for ASCII char 0 - char 127
		inline int tolower(int c)
		{
			if (!::dbj::str::isalpha(c)) return c;
			return (c >= 'A' && c <= 'Z') ? c - 'A' : c;
		}
	}
#pragma endregion 

	/*
	(c) 2017, 2018 by dbj.org
	"zero" time modern C++ versions of str(n)len
	this should speed up any modern C++ code ... perhaps quite noticeably

	standard pointer versions of strlen and strnlen are not used by these two
	which are for arrays only
	note: str(n)len is charr array length  -1 because it does not count the null byte at the end
	note: strnlen is a GNU extension and also specified in POSIX (IEEE Std 1003.1-2008).
	If strnlen is not available for char arrays
	use the dbj::strnlen replacement.

	Inspired by: https://opensource.apple.com/source/bash/bash-80/bash/lib/sh/strnlen.c
	inline size_t strnlen(const CHAR_T *s, size_t maxlen)
	{
	const CHAR_T *e = {};
	size_t n = {};

	for (e = s, n = 0; *e && n < maxlen; e++, n++)
	;
	return n;
	}

	in here we cater for char, wchar_t, char16_t, char32_t
	for details please see https://docs.microsoft.com/en-us/cpp/cpp/char-wchar-t-char16-t-char32-t

	*/
	template<typename T, size_t N>
	constexpr inline size_t strnlen(
		const T(&carr)[N],
		const size_t & maxlen
	)
	{
		static_assert(dbj::is_std_char_v< std::remove_cv_t<T>>,
			"[dbj strnlen] requires only standard chars");

		return dbj::MIN(N, maxlen) - 1;
	}

	template<typename T, size_t N>
	inline size_t strnlen(
		const T(*carr)[N],	const size_t & maxlen
	) = delete;

	/*
	strlen for C++ native char array reference
	*/
	template<typename T, size_t N>
	constexpr inline size_t strlen(	const T(&carr)[N]	)
	{
		static_assert(dbj::is_std_char_v< std::remove_cv_t<T>>,
			"[dbj strlen] requires only standard chars");
		return N - 1;
	}

	template<typename T, size_t N>
	inline size_t strlen(const T(*carr)[N]) = delete;


	/*
	Pointer (to character arrays) support
	note: iosfwd include file contains char_traits we need

	2018 JUL 19  We would not repeat here what UCRT already has done
	namely char and wchar_t versions
	std lib defines strlen for char * and wchr_t *
	*/
#ifdef _MSC_VER
	constexpr inline size_t strlen(const wchar_t  * cp) { return std::char_traits<wchar_t>::length(cp); }
	constexpr inline size_t strlen(const char     * cp) { return std::char_traits<char>::length(cp); }
	constexpr inline size_t strlen(const char16_t * cp) { return std::char_traits<char16_t>::length(cp); }
	constexpr inline size_t strlen(const char32_t * cp) { return std::char_traits<char32_t>::length(cp); }
#else
	// https://godbolt.org/z/L_uhKR
	// apparently not ok for MSVC
	template<typename C>
	inline size_t strlen(const C * cp) 
	{ 
		return std::char_traits<C>::length(cp); 
	}
#endif

#ifdef _MSC_VER
	
	template<typename C >
	struct strnlen_provider final
	{
		using ptr_t = C * ;

		constexpr size_t operator () ( const ptr_t cp_, size_t maxlen) const noexcept
		{
			auto sz_ = std::char_traits<C>::length(cp_);
			return (sz_ > maxlen ? maxlen : sz_);
		}

		template<size_t N>
		constexpr size_t operator () (const C(&)[N]) const noexcept
		{
			return N;
		}
	};

	constexpr inline strnlen_provider<char>       char_strnlen{};
	constexpr inline strnlen_provider<wchar_t>   wchar_strnlen{};
	constexpr inline strnlen_provider<char16_t> char16_strnlen{};
	constexpr inline strnlen_provider<char32_t> char32_strnlen{};

	/*---------------------------------------------------------------*/
	constexpr inline size_t strnlen(const wchar_t * cp, const size_t & maxlen) {
		return wchar_strnlen( const_cast<strnlen_provider<wchar_t>::ptr_t>(cp), maxlen);
	}

	constexpr inline size_t strnlen(const char * cp, const size_t & maxlen) {
		return char_strnlen(const_cast<strnlen_provider<char>::ptr_t>(cp), maxlen);
	}

	constexpr inline size_t strnlen(const char16_t * cp, const size_t & maxlen) {
		return char16_strnlen(const_cast<strnlen_provider<char16_t>::ptr_t>(cp), maxlen);
	}
	constexpr inline size_t strnlen(const char32_t * cp, const size_t & maxlen) {
		return char32_strnlen(const_cast<strnlen_provider<char32_t>::ptr_t>(cp), maxlen);
	}
#else
	template<typename T>
	inline size_t strnlen(const T * cp, const size_t & maxlen) {
		size_t cpl = std::char_traits<T>::length(cp);
		return (cpl > maxlen ? maxlen : cpl);
	}

#endif
	// constexpr string
	// dbj: big note! this class does not own anything, 
	// just points to
	class str_const final
	{
		const char* const p_{ nullptr };
		const std::size_t sz_{ 0 };
	public:

		template<std::size_t N>
		constexpr str_const(const char(&a)[N]) : // ctor
			p_(a), sz_(N - 1) {
		}

		template<std::size_t N>
		constexpr str_const(const char(&a)[N], std::size_t from, std::size_t to) : // ctor
			p_(a + from), sz_(from + to)
		{
			static_assert(to <= N);
			static_assert(from < to);
		}

		// dbj added
		constexpr const char * data() const { return this->p_; }

		constexpr char operator[](const std::size_t & n) const
		{
			return (n <= sz_ ? this->p_[n] : throw std::out_of_range(__func__));
		}

		constexpr std::size_t size() const noexcept { return this->sz_; }

		// dbj added
		constexpr bool friend operator == (const str_const & left, const str_const & right)
		{
			if (left.size() != right.size())
				return false;
			std::size_t index_ = 0, max_index_ = left.size();
			while (index_ < max_index_) {
				if (left[index_] != right[index_]) {
					return false;
				}
				index_ += 1;
			}
			return true;
		}
	}; // str_const

	// https://en.cppreference.com/w/cpp/language/constexpr
	// C++11 constexpr functions had to put everything in a single return statement
	// (C++14 doesn't have that requirement)
	constexpr std::size_t countlower(str_const s, std::size_t n = 0,
		std::size_t c = 0)
	{
		return n == s.size() ? c :
			'a' <= s[n] && s[n] <= 'z' ? countlower(s, n + 1, c + 1) :
			countlower(s, n + 1, c);
	}


	constexpr std::size_t buffer_optimal_size{ 0xFF };

	/*
	Make a string optimized for small sizes
	that is up to arbitrary value of 255
	this makes std::basic_string not to do heap alloc/de-alloc
	for strings up to 255 in length
	Discussion (for example):
	http://www.modernescpp.com/index.php/component/jaggyblog/c-17-avoid-copying-with-std-string-view
	*/
	template <
		typename CT,
		typename string_type = std::basic_string< CT >,
		typename char_type = typename string_type::value_type,
		typename size_type = typename string_type::size_type
	>
		constexpr inline
		// alow only std char types to be used
		std::enable_if_t< dbj::is_std_char_v<CT>, string_type  >
		optimal
		(
			size_type SMALL_SIZE = buffer_optimal_size,
			char_type init_char_
			= static_cast<char_type>('?')
		)
	{
		return string_type(
			SMALL_SIZE,
			init_char_
		);
	}

	/*
	basically I recomend do not use std::string, but for text manipulation(s)
	std::vector<char> use even less, it is crazy slow for simple char buffer-ing
	my preffered buffer type is std::array<>
	2019-02-11 dbj.org
	*/
	using buffer_type = ::std::array<char, buffer_optimal_size >;
	using wbuffer_type = ::std::array<wchar_t, buffer_optimal_size >;

	inline constexpr buffer_type optimal_buffer(void)	{ return buffer_type{ {char(0)} }; }
	inline constexpr wbuffer_type optimal_wbuffer(void) {	return wbuffer_type{ {wchar_t(0)} };	}

	/*
	-------------------------------------------------------------
	need to return std::string since the length is unknown
	at compile time
	*/
	template <
		typename CT,
		typename char_type = std::decay_t<CT>,
		typename return_type = char_type 
	>
		inline
		// alow only char and wchar_t 
		std::enable_if_t< dbj::is_char_v<char_type> || dbj::is_wchar_v<char_type>, char_type *  >
		lowerize(
			CT * from_, CT  const * last_
		)
	{
		assert((from_ != nullptr) && (last_ != nullptr) && (last_ != from_));
#ifdef _DEBUG
		// TODO: how do we know N is really valid?
		// this? https://stackoverflow.com/a/26403920/10870835
		static const auto npos = (size_t)(-1);
		const size_t N = last_ - from_ ;
		assert(N > 0 );
		assert(N < npos );
#endif
		static auto loc = std::locale("");
		// facet of user's preferred locale
		static const std::ctype<CT >	& facet_ 
			= std::use_facet<std::ctype<char_type>>(loc);

		auto the_end_[[maybe_unused]] 
			= facet_.tolower( from_,  last_);
		return from_;
	}
	/*-------------------------------------------------------------
	Can not operate on the view and chage the data 
    */
	template <	typename CT	>
	inline std::basic_string< CT >
	lowerize(std::basic_string_view<CT> view_) = delete;

	/*-------------------------------------------------------------*/
	template <
		typename CT, std::size_t N, typename string_type = std::array<CT,N>
	>
		constexpr inline
		// only char wchar_t
		std::enable_if_t< dbj::is_char_v<CT> || dbj::is_wchar_v<CT>, string_type  >
		lowerize(
			CT(&charr_)[N]
		)
	{
		CT * ptr_to_lowered_ = lowerize(charr_, (charr_ + N));
		std::array<CT, N> rezult_{ {CT(0)} };
		std::copy(charr_, (charr_ + N), rezult_.data());
		return rezult_;
	}


	/// <summary>
	/// ui compare means locale friendly compare
	/// apparently collate id can not be compiled/linked for 
	/// char16_t and char32_t
	/// </summary>
	template<
		bool ignore_case,
		typename CT,
		typename std::enable_if_t< dbj::is_char_v<CT> || dbj::is_wchar_v<CT>, int > = 0
	>
		inline int ui_string_compare
		(
			 CT const * p1,  CT  const * p2
		)
	{
		// the collate type 
		using collate_type = std::collate<CT>;
		// the "C" locale is default 
		static std::locale loc{};
		// get collate facet:
		static const auto & coll = std::use_facet< collate_type >(loc);

		assert(p1 != nullptr);
		assert(p2 != nullptr);
		
		size_t len1, len2;

		if constexpr (dbj::is_char_v<CT>) {
			len1 = std::strlen(p1);
			len2 = std::strlen(p2);
		}
		else {
			len1 = std::wcslen(p1);
			len2 = std::wcslen(p2);
		}

		if constexpr (ignore_case) {
			if constexpr (dbj::is_char_v<CT>) {
				return (::strcmp(p1, p2));
			}
			else {
				return (::wcscmp(p1, p2));
			}
		} else {
			if constexpr (dbj::is_char_v<CT>) {
				return (::_stricmp(p1, p2));
			}
			else {
				return (::_wcsicmp(p1, p2));
			}
		}
	}

	template<
		typename CT,
		typename std::enable_if_t< dbj::is_char_v<CT> || dbj::is_wchar_v<CT>, int > = 0
	>
		inline int ui_string_compare
		(
			CT const * p1, CT  const * p2, bool ignore_case = true
		) {
		if (ignore_case)
			return ui_string_compare<true>(p1,p2);
		return ui_string_compare<false>(p1,p2);
	}
	/// <summary>
	/// is Lhs prefix to Rhs
	/// L must be shorter than R
	/// </summary>
	template < 
		typename CT, 
		typename std::enable_if_t< dbj::is_std_char_v<CT>, int> = 0 
	>
	constexpr inline  bool  
		is_view_prefix_to_view
		(
		std::basic_string_view<CT> lhs, std::basic_string_view<CT> rhs
	    )
	{
		_ASSERTE(lhs.size() > 0);
		_ASSERTE(rhs.size() > 0);

		// "predefined" can not be a prefix to "pre" 
		// opposite is not true
		if (lhs.size() > rhs.size()) return false;

		return dbj::util::equal_(
			lhs.begin(),
			lhs.end(),
			rhs.begin());
	}

	// hard to believe but yes, all done at compile time
	template < typename CT,
		typename string_type = std::basic_string<CT>,
		typename string_view_type = std::basic_string_view<CT>,
		typename std::enable_if_t< dbj::is_std_char_v<CT>, int> = 0 >
		constexpr inline  bool  
		is_prefix
		(
			CT const * lhs, CT const * rhs
		)
	{
		_ASSERTE(lhs != nullptr);
		_ASSERTE(rhs != nullptr);
		string_type ls(lhs), rs(rhs);
		return is_view_prefix_to_view(
			string_view_type{ ls.data(), ls.size() },
			string_view_type{ rs.data(), rs.size() }
		);
	}

	struct __declspec(novtable) backslash final
	{
		template< typename CT, typename std::enable_if_t< dbj::is_std_char_v<CT>, int> = 0 >
		static size_t add(std::basic_string<CT> & path_, CT slash_)
		{
			if (path_.back() != slash_) {
				path_[path_.size() + 1] = slash_;
			}
			return path_.size();
		}

		static auto add(std::string & path_) { return add(path_, '\\'); }
		static auto add(std::wstring & path_) { return add(path_, L'\\'); }
		static auto add(std::u16string & path_) { return add(path_, u'\\'); }
		static auto add(std::u32string & path_) { return add(path_, U'\\'); }
	};

#ifdef DBJ_USE_STD_STREAMS
	inline dbj::wstring_vector
		tokenize(const wchar_t * szText, wchar_t token = L' ')
	{
		dbj::wstring_vector words{};
		std::wstringstream ss;
		ss.str(szText);
		std::wstring item;
		while (getline(ss, item, token))
		{
			if (item.size() != 0)
				words.push_back(item);
		}

		return words;
	}
	// narrow version
	inline dbj::string_vector
		tokenize(const char * szText, char token = ' ')
	{
		dbj::string_vector words{};
		std::stringstream ss;
		ss.str(szText);
		std::string item;
		while (getline(ss, item, token))
		{
			if (item.size() != 0)
				words.push_back(item);
		}

		return words;
	}
#endif

	namespace inner {
		/// <summary>
		/// Try to convert any range made of standard char types
		/// return type should be one of std strings
		/// range is anything that has begin() and end(), and 
		/// value_type typedef as per std containers model
		/// </summary>
		template < typename return_type >
		struct meta_converter final
		{
			/*
			Optimization: if two types are the same 
			just return a copy, sorry: move a copy out
			*/
			return_type operator () (return_type arg)
			{
				return arg;
			}

			template<typename T>
			return_type operator () (T arg)
			{
				if constexpr (dbj::is_range_v<T>) {
					static_assert (
						// arg must have this typedef
						dbj::is_std_char_v< T::value_type >,
						"can not transform ranges not made out of standard char types"
						);
					return { arg.begin(), arg.end() };
				}
				else {
					using actual_type
						= std::remove_cv_t< std::remove_pointer_t<T> >;
					return this->operator()(
						std::basic_string<actual_type>{ arg }
					);
				}
			}
		}; // meta_converter

		   // explicit instantiations
		template struct meta_converter<std::string   >;
		template struct meta_converter<std::wstring  >;
		template struct meta_converter<std::u16string>;
		template struct meta_converter<std::u32string>;

		template < typename T >
		std::ostream & operator << (std::ostream & os, const meta_converter<T> & ws_)
		{
			return os << "\nMeta Converter, return type: " << typeid(T).name() << "\n\n";
		}

	} // inner

} // dbj::str

namespace dbj {
	// all the meta converter INTANCES required / implicit instantiations
	inline dbj::str::inner::meta_converter<std::string   > range_to_string{};
	inline dbj::str::inner::meta_converter<std::wstring  > range_to_wstring{};
	inline dbj::str::inner::meta_converter<std::u16string> range_to_u16string{};
	inline dbj::str::inner::meta_converter<std::u32string> range_to_u32string{};
}

namespace dbj::str {
	template <class T>
	using expr_type = typename std::remove_cv_t<std::remove_reference_t<T>>;

	/*
	remove all instances of a substring found in a string
	call with string view literals for the easiest usage experience
	work for all std char/string/string_view types
	*/
	template <
		typename CT,
		typename string_type = std::basic_string<CT>,
		typename size_type = typename string_type::size_type
	>
		inline
		std::enable_if_t< dbj::is_std_char_v<CT>, string_type >
		remove_all_subs(
			std::basic_string_view<CT> input_,
			std::basic_string_view<CT> pattern
		)
	{
		string_type rezult{ input_.data() };
		size_type n = pattern.length();
		for (size_type j = rezult.find(pattern);
			j != string::npos;
			j = rezult.find(pattern)
			)
		{
			rezult.erase(j, n);
		}
		return rezult;
	}

	template <
		typename CT,
		typename string_type = std::basic_string<CT>,
		typename size_type = typename string_type::size_type
	>
		inline
		std::enable_if_t< dbj::is_std_char_v<CT>, string_type >
		remove_first_sub(
			std::basic_string_view<CT> s, std::basic_string_view<CT> pattern
		) {
		string_type rezult{ s.data() };
		size_type j = rezult.find(pattern.data());
		if (j != string_type::npos) {
			size_type n = pattern.length();
			rezult.erase(j, n);
		}
		return rezult;
	}


	// https://stackoverflow.com/questions/4643512/replace-substring-with-another-substring-c

	template<typename C>
	inline
		std::enable_if_t< dbj::is_std_char_v<C>, std::basic_string<C> >
		replace_inplace
		(
			std::basic_string_view<C>  subject,
			std::basic_string_view<C>  search,
			std::basic_string_view<C>  replace
		)
	{
		std::basic_string<C> input{ (C *)subject.data() };
		size_t pos = 0;
		while ((pos = input.find(search, pos)) != std::string::npos) {
			input.replace(pos, ptrdiff_t(search.length()), replace);
			pos += ptrdiff_t(replace.length());
		}
		return input;
	}

	template<typename C>
	inline
		std::enable_if_t< dbj::is_std_char_v<C>, std::basic_string<C> >
		replace_inplace
		(
			C const * subject,
			C const * search,
			C const * replace
		) {
		return std::basic_string<C>{
			replace_inplace(
				std::basic_string_view<C>{ subject},
				std::basic_string_view<C>{ search },
				std::basic_string_view<C>{ replace })
		};
	}

} // dbj::str

namespace dbj {
/*
NOTE: bellow is some repetition of functionality from above
A bit more classical approach, in a struct that dictates behaviour
How? It has no data but types which must be used and functions operating
on them types only
*/
	template<typename C = char>
	struct str_util {

		typedef C char_type;
		typedef std::basic_string<char_type> string_type;

		static const char_type whitespaces_and_space[]; // { L"\t\n\v\f\r " };

		static string_type to_string(int n)
		{
			return convert_string(std::to_string(n));
		}

		static std::string convert_string(const std::wstring & ws_) {
			return { ws_.begin(), ws_.end() };
		}

		static std::wstring convert_string(const std::string & ss_) {
			return { ss_.begin(), ss_.end() };
		}

		static bool equal_and_found(wchar_t c1, wchar_t c2, const wchar_t * specimen) {
			return c1 == c2 && (nullptr != (wcschr(specimen, c2)));
		}

		static bool equal_and_found(char c1, char c2, const char * specimen) {
			return c1 == c2 && (nullptr != (strchr(specimen, c2)));
		}

		// remove whitespace + space pairs by default
		// or whatever else has to_be_removed
		static string_type  compressor
		(string_type s1, const char_type * to_be_single = whitespaces_and_space)
		{
			string_type s2;
			std::unique_copy(
				s1.begin(),
				s1.end(),
				back_inserter(s2),
				[&](char_type c1, char_type c2)
			{
				// return c1 == c2 && (nullptr != (_tcschr(to_be_single, c2)));
				return equal_and_found(c1, c2, to_be_single);
			}
			);
			return s2;
		}

		static string_type  normalizer
		(string_type str, const char_type * to_be_single = whitespaces_and_space)
		{
			str = compressor(str, to_be_single);
			return trim(str);
		}

		// by default remove all white spaces and a space
		static string_type& ltrim(string_type& str, const string_type& chars = whitespaces_and_space)
		{
			str.erase(0, str.find_first_not_of(chars));
			return str;
		}

		static string_type& rtrim(string_type& str, const string_type& chars = whitespaces_and_space)
		{
			str.erase(str.find_last_not_of(chars) + 1);
			return str;
		}

		static string_type& trim(string_type& str, const string_type& chars = whitespaces_and_space)
		{
			return ltrim(rtrim(str, chars), chars);
		}

		static string_type & simple_replace(string_type & str, char_type to_find, char_type replacement)
		{
			std::replace(str.begin(), str.end(), to_find, replacement);
			return str;
		}

		// replace all found with one given
		template< typename T = char_type, size_t N >
		static string_type & replace_many_one(string_type& str, const T(&many_)[N], char_type one_)
		{
			for (const char_type & ch : many_) {
				str = simple_replace(str, ch, one_);
			}
			return str;
		}

		// replace white spaces and space with underscore to make the input presentable
		// that is: printable to the console 
		static string_type & presentable(string_type& str, char_type one_ = '_')
		{
			return replace_many_one(str, str_util_char::whitespaces_and_space, one_);
		}

		// returns the number of chars replaced
		static int replace(string_type& str, char_type to_find, char_type replacement)
		{
			int accumulator_ = 0;
			std::for_each(str.begin(), str.end(),
				[&](char_type & char_) {
				if (char_ == to_find) {
					accumulator_ += 1;
					char_ = replacement;
				}
			}
			);
			return accumulator_;
		}

		// returns the number of substrings replaced
		static int replace_substring(
			string_type& str,
			const string_type& search,
			const string_type& replace
		)
		{
			size_t pos = 0;
			size_t replacements_ = 0;
			while ((pos = str.find(search, pos)) != string_type::npos) {
				str.replace(pos, search.length(), replace);
				pos += replace.length();
				replacements_ += 1;
			};
			return replacements_;
		}

		static bool char_found(wchar_t c, wchar_t const * text_) { return NULL != wcschr(text_, c); }

		static bool char_found(char    c, char    const * text_) { return NULL != strchr(text_, c); }

		// preserve the input
		// returns the number of chars removed
		static string_type remove_chars(
			string_type& str,
			char_type const * chars_to_remove = whitespaces_and_space)
		{
			int how_many_were_removed = 0;
			string_type output_;
			output_.reserve(str.size());
			// above ctor avoids buffer reallocations in da loop

			for (auto && ch : str)
				if (!char_found(ch, chars_to_remove)) {
					output_.append(1, ch);
					how_many_were_removed++;
				};
			// todo: we can return a pair to have back how_many_were_removed
			return output_;
		}

#if 0 
		// 2019 FEB 26 REMOVED
		// use dbj::fmt suite from the core
		template <size_t BUFSIZ_ = 1024U, typename ... Args>
		static std::wstring
			format(wchar_t const * const message, Args ... args) noexcept
		{
			wchar_t buffer[BUFSIZ_]{};
			auto R = _snwprintf_s(
				buffer,
				BUFSIZ_,
				_TRUNCATE /*_countof(buffer)*/
				, message, (args) ...);
			_ASSERTE(-1 != R);
			return { buffer };
		}

		template <size_t BUFSIZ_ = 1024U, typename ... Args>
		static std::string
			format(char const * const message, Args ... args) noexcept
		{
			char buffer[BUFSIZ_]{};
			auto R = _snprintf_s(
				buffer,
				_countof(buffer),
				_TRUNCATE /*_countof(buffer)*/
				, message, (args) ...);
			_ASSERTE(-1 != R);
			return { buffer };
		}
#endif
	}; // str_util_

	typedef str_util<wchar_t> str_util_wide;
	typedef str_util<char   > str_util_char;

	const str_util_wide::char_type
		str_util_wide::whitespaces_and_space[] = { L"\t\n\v\f\r " };
	const str_util_char::char_type
		str_util_char::whitespaces_and_space[] = { "\t\n\v\f\r " };


	namespace str {

		/*
		Actually a left pad where default maxlen is 12
		*/
		inline auto string_pad(std::string s_, char padchar = ' ', size_t maxlen = 12) {
			return s_.insert(0, maxlen - s_.length(), padchar);
		};

		inline auto string_pad(int number_, char padchar = ' ', size_t maxlen = 12) {
			return string_pad(std::to_string(number_), padchar, maxlen);
		};

		/*
		text line of chars, usage
		constexpr auto line_ = c_line();
		constexpr auto line_2 = c_line<60>('=');
		*/
		template<size_t size = 80>
		constexpr inline std::string_view  char_line(const char filler = '-')
		{
			// feed the static string view ctor with data from a
			// static std::array
			static std::string_view the_line_(
				[&]
			{
				static std::array< char, size + 1 > array_{};
				array_.fill(filler);
				array_[size] = '\0';
				return array_.data();
			}()
				);
			return the_line_;
		};

		// create text line of 80 chars '-' by default
		// once called can not be changed ;)
		// there is also dbj::util::char_line
		template< size_t N = 80, typename ARF = char(&)[N], typename ARR = char[N] >
		const ARF line(char fill_char = '-') 
		{
			auto set = [&]() -> ARF {
				static ARR arr;
				auto rz = std::memset(arr, fill_char, N);
				return arr;
			};

			// allocate array on stack once
			static ARF arr_ = set();
			return arr_;
		}


		/*
		bellow works for all the std:: string types and string view types
		but it also works for char pointers and wchar_t pointers
		*/
		template< typename C >
			inline auto starts_with(
				std::basic_string_view<C> val_,
				std::basic_string_view<C> mat_
			)
		{
			return 0 == val_.compare(0, mat_.size(), mat_);
		}

// https://github.com/fenbf/StringViewTests/blob/master/StringViewTest.cpp
// works on pointers rather than iterators
// code by JFT
// DBJ: changed argument types to be string_view, not string
// now this is even faster
// DBJ: made it so that delims are the standard white space chars

			inline ::dbj::string_vector fast_string_split(
				const string_view & str,
				const string_view & delims = " \t\v\n\r\f"
			)
			{
				::dbj::string_vector output;
				//output.reserve(str.size() / 2);

				for (
					auto first = str.data(), second = str.data(), last = first + str.size();
					second != last && first != last;
					first = second + 1)
				{
					second = std::find_first_of(
						first, last, std::cbegin(delims), std::cend(delims)
					);

					if (first != second)
						output.emplace_back(first, second);
				}

				return output;
			}

	} // str
} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
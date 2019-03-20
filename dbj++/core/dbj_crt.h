#pragma once

// #include <sysinfoapi.h>
// #include <strsafe.h>
// #include <algorithm>
// #include <cstddef>
// #include <cwctype>
// #include <cctype>
// #include <string>
// #include <vector>
// #include <map>
// #include <algorithm>

#include <cassert>
#include <string_view>
#include <charconv>
#include <system_error>
#include <array>

#include "dbj_traits.h"
#ifndef __clang__
#ifndef _MSC_VER
#error dbj++  requires Visual C++ 
#endif // !_MSC_VER
#if _MSC_VER < 1911
#error dbj++ requires Visual C++ 14.1 or better
#endif
#endif

#if (!defined(UNICODE)) || (! defined(_UNICODE))
#error dbj++ requires UNICODE builds
#endif

#ifdef _SCL_SECURE_NO_WARNINGS
#error dbj++ can not be used with  _SCL_SECURE_NO_WARNINGS defined
#endif

//#ifdef __cpp_coroutines
//#pragma message ( __FILE__ " -- coroutines available in this build")
//#else
//#pragma message (__FILE__ " -- no coroutines in this build ...")
//#endif

/* avoid macros as much as possible */
// #ifdef NOMINMAX
inline const auto & MIN = [](const auto & a, const auto & b) 
  constexpr -> bool { return (a < b ? a : b); };
inline const auto & MAX = [](const auto & a, const auto & b) 
  constexpr -> bool { return (a > b ? a : b); };
// #else
// #error Be sure you have #define NOMINMAX placed before including windows.h !
// #endif


// NOTE: do not have a space after a macro name and before the '(' !!
#ifndef DBJ_STRINGIFY	
#define DBJ_STRINGIFY_(s) #s
#define DBJ_STRINGIFY(s) DBJ_STRINGIFY_(s)
#define DBJ_EXPAND(s) DBJ_STRINGIFY(s)
#define DBJ_CONCAT_IMPL( x, y ) x##y
#define DBJ_CONCAT( x, y ) DBJ_CONCAT_IMPL( x, y )

#define DBJ_EXP_(s) #s
#define DBJ_EXP(s) DBJ_EXP_(s)
#endif

/*
this is for variables only
example
long DBJ_MAYBE(var) {42L} ;
after expansion:
long var [[maybe_unused]] {42L} ;
*/
#define DBJ_MAYBE(x) x [[maybe_unused]]
/* just an unused variable but used expression */
#define DBJ_USD(x) auto DBJ_CONCAT( unused_ , __COUNTER__ ) [[maybe_unused]] = (x)


/*
for variables and expressions
guaranteed no evaluation
guaranteed zero bytes overhead
standard c++
works in any space, example here
https://godbolt.org/z/jGC98L
*/
#define DBJ_NOUSE(...) static_assert( (noexcept(__VA_ARGS__),true) );

// https://www.boost.org/doc/libs/1_35_0/boost/preprocessor/stringize.hpp
#    define BOOST_PP_STRINGIZE(text) BOOST_PP_STRINGIZE_A((text))
#    define BOOST_PP_STRINGIZE_A(arg) BOOST_PP_STRINGIZE_I arg
#    define BOOST_PP_STRINGIZE_I(text) #text
//

#ifndef DBJ_COMPANY	

#ifdef _DEBUG
#define DBJ_BUILD "DBJ*Debug"
#else
#define DBJ_BUILD "DBJ*Release"
#endif 

#define DBJ_COMPANY "DBJ.Systems Ltd."
#define DBJ_YEAR (__DATE__ + 7)
#define DBJ_BUILD_STAMP "(c) " __DATE__ " by " DBJ_COMPANY "| " DBJ_BUILD ": [" __DATE__ "][" __TIME__ "]" 
#endif
#define DBJ_ERR_PROMPT(x) \
__FILE__ "(" DBJ_EXPAND(__LINE__) ") -- " __FUNCSIG__ " -- " x " -- "

// 
#define DBJ_CHECK_IF static_assert

/*
DBJ preffered concept to code standard C++ string constants
is to use string_view

#include <string_view>
using namespace
::std::literals::string_view_literals;

	constexpr inline auto
	   compiletime_string_view_constant
		= "compile time"sv ;

as evident this also works at compile time

static_assert(compiletime_string_view_constant == "compile time" );

*/

namespace dbj {

#ifndef ssize_t
#define ssize_t signed long
#endif

	// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1227r1.html
	// dbj: this is dubious
	inline ssize_t ssize( std::size_t signed_size ) {
		return static_cast<ssize_t>(signed_size);
	}

	using namespace ::std;
	using namespace ::std::string_view_literals;

	// probably the most used types
	using wstring_vector = vector<wstring>;
	using string_vector  = vector<string>;

	constexpr inline auto LINE    (){ return "--------------------------------------------------------------------------------"sv; };
	constexpr inline auto COMPANY (){ return "DBJ.Systems Ltd."sv; };
	constexpr inline auto YEAR    (){ return std::string_view{ (__DATE__ + 7) }; };

	inline wchar_t const * app_base_name() noexcept {
		auto initor = []() {
			std::wstring base_name(__wargv[0]);
			base_name.erase(0,
				1 + base_name.find_last_of(L"\\")
			);
			return base_name;
		};
		static auto basename_ = initor();
		return basename_.c_str();
	}


	namespace nano
	{
		template<typename T, typename F>
		inline T
			transform_to(F str) noexcept
		{
			if constexpr (!is_same_v<T, F>) {
				if (str.empty()) return {};
				return { std::begin(str), std::end(str) };
			}
			else {
				// T and F are the same type
				// thus just move a copy
				return str;
			}
		};
		// no native pointers please
		// use literals
		template<typename T, typename F>
		inline T transform_to(F *) = delete;

		[[noreturn]]
		inline void terror
		(char const * msg_, char const * file_, const unsigned line_)
		{
			assert(msg_ != nullptr); assert(file_ != nullptr); assert(line_ > 0);
			::fprintf(stderr, "\n\ndbj++ Terminating ERROR:%s\n\n%s (%d)\n\n", msg_, file_, line_);
#ifdef _DEBUG
#ifdef _MSC_VER
			::system("@echo.");
			::system("@pause");
			::system("@echo.");
#endif
#endif
			exit(EXIT_FAILURE);
		}

	} // nano

// decades old VERIFY macro
#define DBJ_VERIFY_(x, file, line ) if (false == (x) ) ::dbj::nano::terror( "  " #x " , failed at", file, line )
#define DBJ_VERIFY(x) DBJ_VERIFY_(x,__FILE__,__LINE__)


	/* 512 happpens to be the POSIX BUFSIZ */
#ifndef BUFSIZ
#define BUFSIZ 512
#endif

	constexpr inline size_t BUFSIZ_ = BUFSIZ * 2 * 4; // 4096

	/// <summary>
	/// http://en.cppreference.com/w/cpp/types/alignment_of
	/// please make sure alignment is adjusted as 
	/// necessary
	/// after calling these functions
	/// </summary>
	namespace heap {

		template<typename T>
		inline T * alloc(size_t size_ = 0) {
			T * rez_ = new T;
			return rez_;
		}

		template<typename T>
		inline bool free(T * ptr_) {
			_ASSERTE(ptr_);
			delete ptr_;
			ptr_ = nullptr;
			return true;
		}

#ifdef DBJ_WIN

		template<typename T>
		inline T * alloc_win(size_t size_ = 0) {
			T * rez_ = (T*)CoTaskMemAlloc(sizeof(T));
			_ASSERTE(rez_ != nullptr);
			return rez_;
		}

		template<typename T>
		inline bool free_win(T * ptr_) {
			_ASSERTE(ptr_);
			CoTaskMemFree((LPVOID)ptr_);
			ptr_ = nullptr;
			return true;
		}

#endif // DBJ_WIN

	}
	/*
	Core algo from http://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign

	standard c++ will compile this, for any T, that 
	is applicable to required operators
	return value is trigraph -1,0,+1
	result will be constexpr on POD types
	*/
	template<typename T>
	 inline constexpr auto  sign ( T const & v) 
	 {
		return (v > 0) - (v < 0); // -1, 0, or +1
	 };

// if the argument of the DBJ_ARR_LEN macro is a pointer, 
// code won't compile 
#define DBJ_ARR_LEN(str) (::dbj::countof(str)) 

	template < typename T, size_t N > 
	  inline constexpr size_t 
		countof(T const (&)[N]) { 
		  return N; 
	  }

} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"

/* standard suffix for every dbj header */
#pragma comment( user, DBJ_BUILD_STAMP ) 
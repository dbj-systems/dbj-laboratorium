#pragma once

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
#ifdef MIN
#undef MIN
#pragma message (__FILE__ " -- MIN undefined ...")
#endif // MIN
#ifdef MAX
#undef MAX
#pragma message (__FILE__ " -- MAX undefined ...")
#endif // MAX

inline const auto & MIN = [](const auto & a, const auto & b) 
  constexpr -> bool { return (a < b ? a : b); };
inline const auto & MAX = [](const auto & a, const auto & b) 
  constexpr -> bool { return (a > b ? a : b); };
// #else
// #error Be sure you have #define NOMINMAX placed before including windows.h !
// #endif

#ifdef _MSC_VER
#define DBJ_ASSERT(x) _ASSERTE(x)
#define DBJ_VERIFY(x) _ASSERT_AND_INVOKE_WATSON(x)
#else
#define DBJ_ASSERT(x) assert(x)
#define DBJ_VERIFY(x) do{ if(!(x) { perror(_DBJ_STRINGIZE_(x)); exit(0);} }while(0)
#endif

#ifdef _MSC_VER
// __LINE__ is not contexp in MSVC because of "edit and continue" feature of VStudio
// apparently not a bug but a feature ...
// https://developercommunity.visualstudio.com/content/problem/195665/-line-cannot-be-used-as-an-argument-for-constexpr.html
#define _DBJ_CONSTEXPR_LINE long(_DBJ_CONCATENATE(__LINE__,U)) 
#else
#define _DBJ_CONSTEXPR_LINE __LINE__
#endif

// for iostreams related functionality
// although, why don't we just declare we do no use
// iostreams at all?
// dbj 2019-08-13
// #define DBJ_USES_STREAMS

/*
from vcruntime.h
*/
#define _DBJ_STRINGIZE(x) _DBJ_STRINGIZE_(x)
#define _DBJ_STRINGIZE_(x) #x

#define _DBJ_WIDE(s) _DBJ_WIDE_(s)
#define _DBJ_WIDE_(s) L ## s

#define _DBJ_CONCATENATE(a_, b_) _DBJ_CONCATENATE_IMPL_(a_, b_)
#define _DBJ_CONCATENATE_IMPL_(a_, b_) a_##b_

#define _DBJ_EXPAND(s) _DBJ_EXPAND_IMPL_(s)
#define _DBJ_EXPAND_IMPL_(s) s

// NOTE: do not have a space after a macro name and before the '(' !!
#if 0
#ifndef DBJ_STRINGIFY	
#define DBJ_STRINGIFY_(s) #s
#define DBJ_STRINGIFY(s) DBJ_STRINGIFY_(s)
#define DBJ_EXPAND(s) DBJ_STRINGIFY(s)
#define DBJ_CONCAT_IMPL( x, y ) x##y
#define DBJ_CONCAT( x, y ) DBJ_CONCAT_IMPL( x, y )

#define DBJ_EXP_(s) #s
#define DBJ_EXP(s) DBJ_EXP_(s)
#endif
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

#if 0
// https://www.boost.org/doc/libs/1_35_0/boost/preprocessor/stringize.hpp
#    define BOOST_PP_STRINGIZE(text) BOOST_PP_STRINGIZE_A((text))
#    define BOOST_PP_STRINGIZE_A(arg) BOOST_PP_STRINGIZE_I arg
#    define BOOST_PP_STRINGIZE_I(text) #text
//
#endif

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

#ifndef DBJ_ERR_PROMPT
// is this resilient or what?
inline auto dbj_err_prompt_and_msg 
( 
	char const * file_, 
	const unsigned line_, 
	char const * msg_
) 
{
	_ASSERTE(file_ && msg_);
	auto line_to_s = [](int line_no_) {
		static std::array<char, 64> buf_{ {0} };
		buf_.fill(0);
		std::snprintf(buf_.data(), 64U, "%d", line_no_);
		return buf_.data();
	};
	::dbj::sync::lock_unlock locker_;
	::std::string text_( BUFSIZ * 4, char(0) );
	text_ = msg_;
	text_.append(" -- ")
		.append(file_)
		.append("(")
		.append(line_to_s(line_))
		.append(") ");

	struct to_cp final {
		mutable std::string data;
		operator char const * () const {
			return data.c_str(); 
		}
		operator std::string_view () const {
			return { data.c_str() };
		}
	};

	return to_cp{ text_ };
};
// note: this macro is used a lot
#define DBJ_ERR_PROMPT(x) (char const *)dbj_err_prompt_and_msg ( __FILE__, __LINE__, x )
#endif // DBJ_ERR_PROMPT

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

	// this is the folder under which all the dbj apps are 
	// holding ini, log files and a such
	// under %programdata% on a local windows machine
	constexpr inline const auto dbj_programdata_subfolder = "dbj";
	// constexpr inline const auto DBJ_LOCAL_FOLDER = "dbj";

	inline wchar_t const * app_base_name() noexcept {
		auto initor = []() {
			_ASSERTE(__wargv);
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
			transform_to(F from_) noexcept
		{
			if constexpr (!is_same_v<T, F>) {
				// if (str.empty()) return {};
				return { std::begin(from_), std::end(from_) };
			}
			else {
				// T and F are the same type
				// thus just move a copy
				return from_;
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

// decades old VERIFY macro, in a new clothing
#define DBJ_TERROR_IMPL_(x, file, line ) if (false == (x) ) ::dbj::nano::terror( "  " #x " , failed at", file, line )
#define DBJ_TERROR(x) DBJ_TERROR_IMPL_(x,__FILE__,__LINE__)


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

	//	 do not try this at home. ever.
	extern "C"	inline void	secure_reset(void* s, size_t n) noexcept
	{
		volatile char* p = (char*)s;
		while (n--)* p++ = 0;
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

	  inline char* basename(
		  char* full_path,
		  bool remove_suffix = true,
		  char delimiter = '\\')
	  {
		  assert(full_path && (full_path[0] != char(0)));
		  char* base_ = strrchr(full_path, delimiter);
		  base_ = (base_ == NULL ? full_path : base_);
		  if (remove_suffix == false) return base_;
		  char* dot_pos = strchr(base_, '.');
		  if (dot_pos)* dot_pos = char(0);
		  return base_ + 1;
	  }

} // dbj



/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"

/* standard suffix for every dbj header */
#pragma comment( user, DBJ_BUILD_STAMP ) 
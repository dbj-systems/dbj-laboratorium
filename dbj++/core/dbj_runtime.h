#pragma once

// REMINDER: be sure you do not have a space after a macro name and before the '(' !!

#if (!defined(UNICODE)) || (! defined(_UNICODE))
#error dbj++ requires UNICODE builds
#endif

#ifdef _SCL_SECURE_NO_WARNINGS
#error dbj++ can not be used with  _SCL_SECURE_NO_WARNINGS defined
#endif

#define DBJ_PP "dbj++"

// safer is slower
// make us aware
#define DBJ_SAFER_IS_SLOWER (1==1)

#if DBJ_SAFER_IS_SLOWER
#define DBJ_SAFE(x) x
#pragma message("\n\ndbj++ safe and slower build\n\n")
#else
#define DBJ_SAFE(x) __noop
#pragma message("\n\ndbj++ faster and maybe more dangerous build\n\n")
#endif

#ifdef _MSC_VER
#define DBJ_ASSERT(x) DBJ_SAFE( _ASSERTE(x) )
#define DBJ_VERIFY(x) DBJ_SAFE( _ASSERT_AND_INVOKE_WATSON(x) )
#else
#define DBJ_ASSERT(x) DBJ_SAFE( assert(x) )
#define DBJ_VERIFY(x) DBJ_SAFE( do{ if(!(x) { perror(_DBJ_STRINGIZE_(x)); exit(0);} }while(0) )
#endif

//#ifdef __cpp_coroutines
//#pragma message ( __FILE__ " -- dbj++ coroutines available in this build")
//#else
//#pragma message (__FILE__ " -- dbj++ no coroutines in this build ...")
//#endif

/* avoid macros as much as possible */
// #ifdef NOMINMAX
#ifdef MIN
#undef MIN
#pragma message (__FILE__ " -- MIN macro is made to be  undefined ...")
#endif // MIN
#ifdef MAX
#undef MAX
#pragma message (__FILE__ " -- MAX macro is made to be  undefined ...")
#endif // MAX

inline const auto& MIN = [](const auto& a, const auto& b)
constexpr -> bool { return (a < b ? a : b); };
inline const auto& MAX = [](const auto& a, const auto& b)
constexpr -> bool { return (a > b ? a : b); };

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

/*
this is for variables only
example
long DBJ_MAYBE(var) {42L} ;
after expansion:
long var [[maybe_unused]] {42L} ;
*/
#define DBJ_MAYBE(x) x [[maybe_unused]]
/* just an unused variable but used expression */
#define DBJ_MAYBE_VAR(x) auto DBJ_CONCAT( unused_ , __COUNTER__ ) [[maybe_unused]] = (x)


/*
for variables and expressions
guaranteed no evaluation
guaranteed zero bytes overhead
standard c++
works in any space, example here
https://godbolt.org/z/jGC98L
*/
// #define DBJ_UNUSED(...) static_assert( (noexcept(__VA_ARGS__),true) );

// pull out all the brakes :) 
#ifdef _unused
#error _unused is already defined somewhere ...
#else
#define _unused(...)  static_assert( noexcept( __VA_ARGS__ , true ) )
#endif

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
	char const* file_,
	const unsigned line_,
	char const* msg_
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
	::std::string text_(BUFSIZ * 4, char(0));
	text_ = msg_;
	text_.append(" -- ")
		.append(file_)
		.append("(")
		.append(line_to_s(line_))
		.append(") ");

	struct to_cp final {
		mutable std::string data;
		operator char const* () const {
			return data.c_str();
		}
		operator std::string_view() const {
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
	inline ssize_t ssize(std::size_t signed_size) {
		return static_cast<ssize_t>(signed_size);
	}

	using namespace ::std;
	using namespace ::std::string_view_literals;

	constexpr inline auto LINE() { return "--------------------------------------------------------------------------------"sv; };
	constexpr inline auto COMPANY() { return "DBJ.Systems Ltd."sv; };
	constexpr inline auto YEAR() { return std::string_view{ (__DATE__ + 7) }; };

	// this is the folder under which all the dbj apps are 
	// holding ini, log files and a such
	// under %programdata% on a local windows machine
	constexpr inline const auto dbj_programdata_subfolder = "dbj";
	// constexpr inline const auto DBJ_LOCAL_FOLDER = "dbj";

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
		inline T transform_to(F*) = delete;

		[[noreturn]]
		inline void terror
		(char const* msg_, char const* file_, const unsigned line_)
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
	constexpr inline size_t BUFSIZ_ = BUFSIZ * 2 * 4; // 4096

	/// <summary>
	/// http://en.cppreference.com/w/cpp/types/alignment_of
	/// please make sure alignment is adjusted as 
	/// necessary
	/// after calling these functions
	/// </summary>
	namespace heap {

		template<typename T>
		inline T* alloc(size_t size_ = 0) {
			T* rez_ = new T;
			return rez_;
		}

		template<typename T>
		inline bool free(T* ptr_) {
			_ASSERTE(ptr_);
			delete ptr_;
			ptr_ = nullptr;
			return true;
		}

#ifdef DBJ_WIN

		template<typename T>
		inline T* alloc_win(size_t size_ = 0) {
			T* rez_ = (T*)CoTaskMemAlloc(sizeof(T));
			_ASSERTE(rez_ != nullptr);
			return rez_;
		}

		template<typename T>
		inline bool free_win(T* ptr_) {
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
	inline constexpr auto  sign(T const& v)
	{
		return (v > 0) - (v < 0); // -1, 0, or +1
	};

	// if the argument of the DBJ_ARR_LEN macro is a pointer, 
	// code won't compile 
#define DBJ_ARR_LEN(str) (::dbj::countof(str)) 

	template < typename T, size_t N >
	inline constexpr size_t countof(T const (&)[N]) { return N; };

	/*
	non UNICODE version
	*/
	extern "C" inline char* dbj_basename(
		char* full_path,
		bool remove_suffix = true,
		char delimiter = '\\')
	{
		assert(full_path && (full_path[0] != char(0)));
		char* base_ = ::strrchr(full_path, delimiter);
		base_ = (base_ == NULL ? full_path : base_);
		if (remove_suffix == false) return base_;
		char* dot_pos = strchr(base_, '.');
		if (dot_pos)* dot_pos = char(0);
		return base_ + 1;
	}

	/*
	1. use wmain()
	2. call this preferably not before wmain()
	*/
	inline wchar_t const* app_base_name() noexcept {
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

#pragma region very core type traits

	/* example usage:
	compiles:
template<unsigned K>
using ascii_ordinal_compile_time = ::dbj::inside_t<unsigned, K, 0, 127>;

constexpr auto compile_time_ascii_index = ascii_ordinal_compile_time<64>() ;
	fails:
'std::enable_if_t<false,std::integral_constant<unsigned int,164>>' : Failed to specialize alias template
 constexpr auto compile_time__not_ascii_index = ascii_ordinal_compile_time<164>() ;
*/
	template< typename T, T X, T L, T H>
	using inside_inclusive_t =
		::std::enable_if_t< (X <= H) && (X >= L),
		::std::integral_constant<T, X> >;

	template< typename T, T X, T L, T H>
	inline constexpr bool  inside_inclusive_v = inside_inclusive_t<T, X, L, H>();

	/*
		static_assert(  dbj::is_any_same_as_first_v<float, float, float> ) ;

	fails, none is same as bool:
		static_assert(  dbj::is_any_same_as_first_v<bool,  float, float>  );
	*/
	template<class _Ty,
		class... _Types>
		inline constexpr bool is_any_same_as_first_v
		= ::std::disjunction_v<::std::is_same<_Ty, _Types>...>;
#pragma endregion

	//  https://en.wikipedia.org/wiki/Proof_that_22/7_exceeds_π
	// https://www.wired.com/story/a-major-proof-shows-how-to-approximate-numbers-like-pi/
	constexpr inline auto DBJ_PI = 104348 / 33215;

	/*
	repeat has no meaning if top value is less then 1
	*/
	struct dbj_repeat_value {
		size_t v_{ 1 };
		dbj_repeat_value(size_t arg_)
			: v_{ arg_ } {
			if ((arg_ < 1) || (arg_ > UINT_MAX))
				throw std::runtime_error("\n\n" __FILE__ "(" _DBJ_STRINGIZE(__LINE__) ")\n\ndbj_repeat_top_value out of bounds\n\n");
		}
		dbj_repeat_value() = delete;

		operator size_t const& () noexcept { return v_; };
		operator size_t const& () const noexcept { return v_; };
	};

	/*
	this macro is actually superior solution to the repeat template function bellow
	dbj_repeat_value(N) throws runtime_error if N <  1 or N > UINT_MAX

	_dbj_repeat_counter is reused for each macro expansion into for(;;){} of course ...
	*/
#define DBJ_REPEAT(N) for(size_t _dbj_repeat_counter = 0 ; _dbj_repeat_counter < ::dbj::dbj_repeat_value(N); _dbj_repeat_counter++)

	// pull out all the brakes
#ifdef repeat
	#error repeat is already defined somewhere ...
#else
	#define repeat(N) for(size_t _dbj_repeat_counter = 0 ; _dbj_repeat_counter < ::dbj::dbj_repeat_value(N); _dbj_repeat_counter++)
#endif


	/*
	usage:
		DBJ_REPEAT(42) { printf("\n42 lines of text"); }
	or
		DBJ_REPEAT(42) { printf("\n%d lines of text", _dbj_repeat_counter ); }


	template< typename CALLABLE_, typename ... Args  >
	inline void repeat( dbj_repeat_value N, CALLABLE_ const& fun_ , Args ... args )
	{
		for (size_t _dbj_repeat_counter = 0; _dbj_repeat_counter < N; _dbj_repeat_counter++) {
			fun_(_dbj_repeat_counter, args ...  );
		}
	}

	following throws runtime_error but *after* random_word(word).data() is executed
	so there is no point of arguments to the callback sent
	they will be executed before repeat value is checked

	dbj::repeat(0,
		[&](auto _dbj_repeat_counter) {
			::std::wprintf(L"\n %Z \t random word:\t '%s'", _dbj_repeat_counter, the_random_word);
		}
		, random_word(word).data()
	);
*/

/*
modern solution usage, ugly as hell:

repeat(42, [&](auto arg_) { printf("\n%s", arg_ );} , "42 lines of text") ;
*/

} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_license.h"

/* standard suffix for every dbj header */
#pragma comment( user, DBJ_BUILD_STAMP ) 
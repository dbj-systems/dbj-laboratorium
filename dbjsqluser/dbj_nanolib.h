#pragma once
#include <cstdio>
#include <vector>
#include <string_view>

#ifdef _MSVC_LANG
#if _MSVC_LANG < 201402L
#error "C++17 required ..."
#endif
#endif

namespace dbj::nanolib {

	using namespace std;


	[[noreturn]] inline void dbj_terror(const char* msg_, const char* file_, const int line_)
	{
		_ASSERTE(msg_ && file_ && line_);
		std::fprintf(stderr, "\n\ndbj++ini Terminating error:%s\n%s (%d)", msg_, file_, line_);
		::exit(EXIT_FAILURE);
	}

#ifndef DBJ_VERIFY
#define DBJ_VERIFY_(x, file, line ) if (false == x ) ::dbj::nanolib::dbj_terror( #x ", failed", file, line )
#define DBJ_VERIFY(x) DBJ_VERIFY_(x,__FILE__,__LINE__)
#endif

#pragma region vector char_type buffer

constexpr inline std::size_t DBJ_64KB = UINT16_MAX;
/*
for runtime buffering the most comfortable and in the same time fast
solution is vector<char_type>

only unique_ptr<char[]> is faster than vector of  chars, by a margin
*/
struct v_buffer final {

	using buffer_type		=  std::vector<char>;

	static
		buffer_type make(size_t count_)
	{
		_ASSERTE(count_ > 0);
		_ASSERTE(DBJ_64KB >= count_);
		buffer_type retval_(count_ + 1, char(0));
		return retval_;
	}

	static
		buffer_type make(std::basic_string_view<char> sview_)
	{
		_ASSERTE(sview_.size() > 0);
		_ASSERTE(DBJ_64KB >= sview_.size());
		buffer_type retval_(sview_.begin(), sview_.end());
		// terminate!
		retval_.push_back(char(0));
		return retval_;
	}

}; // v_buffer
#pragma endregion

inline v_buffer::buffer_type safe_strerror( int errno_ ) 
{
	v_buffer::buffer_type buffy_ = v_buffer::make(BUFSIZ);
	if (0 != strerror_s(buffy_.data(), buffy_.size(), errno_))
		dbj_terror("strerror_s failed", __FILE__, __LINE__);
	return buffy_;
}

	/*
	first arg has to be stdout, stderr, etc ...
	*/
#define DBJ_FPRINTF(...) \
	do { if (errno_t result_ = ::fprintf(__VA_ARGS__); result_ < 0) \
		::dbj::nanolib::dbj_terror( ::dbj::nanolib::safe_strerror( result_ ).data(), __FILE__, __LINE__); } while(false)

#define DBJ_ERR_PROMPT(x) ( __FILE__ "(" _CRT_STRINGIZE( __LINE__ ) ") " _CRT_STRINGIZE( x ) )

	/*
	this is for variables only
	example
	long DBJ_MAYBE(var) {42L} ;
	after expansion:
	long var [[maybe_unused]] {42L} ;
	*/
#define DBJ_MAYBE(x) x [[maybe_unused]]

} // dbj::nanolib


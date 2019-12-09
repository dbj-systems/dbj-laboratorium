#pragma once
#ifndef	_DBJ_PP_VALSTAT
#define	_DBJ_PP_VALSTAT
/*
99% of "things" dbj needs to make valstat usefull for them
are here
*/

#include <valstat>
#include <string>
#include <system_error>

#include "dbj_runtime.h"

namespace dbj
{
	/*
	dbj -- the imagined company
	this type alias is all they have and need to use std::valstat
	this makes them fully std::valstat compliant
	this also makes other people better understand their API's

	NOTE: std::string is not the most performant json
		  string implementation. dbj, they usually use vector<char> or even
		  unique_ptr<char[]>
		  that issue is largely mitigated by not using strings
		  but handles to the strings registry as the status type
	*/
	template< typename T>
	using valstat = std::valstat<T, std::string >;

#ifdef VALSTAT_DIAGNOSTICS
	// descriptive output
	// the  verbose valstat consuming
	// for testing purposes only
	// dbj usualy do not use <iostream> in a production code
	template<typename T>
	inline std::ostream& operator << (std::ostream& os, const dbj::valstat<T> vt)
	{
		os << "\nvalstat state: ";
		if (!vt.value && vt.status) os << "ERROR";
		if (vt.value && !vt.status) os << "OK";
		if (vt.value && vt.status) os << "INFO";
		if (!vt.value && !vt.status) os << "EMPTY";

		os << " , content: \n{";
		if (vt.value)
			os << "\n { value: " << *vt.value << " }";
		else
			os << "\n { value: empty }";
		os << " ,";
		if (vt.status)
			os << "\n { status: " << *vt.status << " }";
		else
			os << "\n { status: empty }";
		return os << "\n}\n";
	}
#endif // VALSTAT_DIAGNOSTICS

	/// <summary>
	/// fact of C++ computing is POSIX error codes are going to stay
	/// thus dbj have decalred the following valstat type too
	/// valstat_bj_status.h contains message
	/// creation from std::errc, part of <system_error>
	/// namespace dbj::posix {
	///		inline std::string errc_to_message(std::errc posix_err_code);
	/// }
	/// used only when needed. 
	/// 
	/// Thus dbj pass the std::errc arround and get to its message
	/// only if and when needed
	/// 
	/// </summary>
	template<typename T>
	using posix_vs = std::valstat<T, std::errc >;
}

/// <summary>
/// dbj valstat status helpers
/// </summary>
namespace dbj {

	using namespace std;

	/*
	it turns out status as a string sub-concept allows for total
	decoupling. dbj have found they do not need to pre-declare valstat
	types for different status types too

	Example:

	valstat<bool> fun ( bool arg_ ) {

	if ( is_win32_error() )
		return {{}, { make_win32_status( GetLastError(), __FILE__, __LINE__ ) }};

	if ( is_posix_error() )
		return {{}, { make_posix_status( std::errc::ENOMEM, __FILE__, __LINE__ ) }} ;

		return {{ arg_ }, {}};
		}

	bellow is just a message+file+line status, for sampling the valstat
	it is in a JSON format as every other status message
	*/

	using v_buffer = dbj::vector_buffer<char> ;

	inline std::string
		make_status(const char* file, long line, const char* time_stamp, const char* msg = nullptr)
	{
		auto nix_path = v_buffer::replace(v_buffer::format("%s", file), '\\', '/');
		v_buffer::buffer_type buffy = v_buffer::format(
			R"({ "message" : "%s", "file" : "%s", "line" : %d, "timestamp" : "%s" })",
			(msg ? msg : "unknown"), nix_path.data(), line, time_stamp);

		return  { buffy.data() };
	}

	namespace posix {
		inline std::string e2m (std::errc posix_err_code)
		{
			::std::error_code ec = std::make_error_code(posix_err_code);
			return ec.message();
		};

		// consume immediately
		inline char const* e2s(std::errc posix_err_code)
		{
			static std::string anchor_{};
			anchor_ = e2m(posix_err_code);
			return anchor_.c_str();
		};

	} // posix


	// move to WIN32 using components
	//
	//namespace win32 {
	//	/* win32 error code as a (strong) type */
	//	struct error_code;
	//	struct error_code
	//	{
	//		int v{ 0 };
	//		error_code() : v(::GetLastError()) { ::SetLastError(0); }
	//	};

	//	constexpr inline bool is_ok(error_code const& ec_) { return ec_.v == 0; }

	//	/* Last WIN32 error, message */
	//	inline std::string error_message(int code = 0)
	//	{
	//		std::error_code ec(
	//			(code ? code : ::GetLastError()),
	//			std::system_category()
	//		);
	//		::SetLastError(0); //yes this helps
	//		v_buffer::buffer_type buffy = v_buffer::format("%s", ec.message().c_str());
	//		return { buffy.data() };
	//	}

	//	inline auto code_to_message(win32::error_code code) -> std::string
	//	{
	//		if (code.v)
	//			return error_message(code.v);
	//		return { "No error" };
	//	};
	//} // win32

} // ns

#endif //	_DBJ_PP_VALSTAT

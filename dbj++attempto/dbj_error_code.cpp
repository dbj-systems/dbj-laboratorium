#include "pch.h"

/* 
it is a standard way how to do it so
we will copy the design of std:: ios error coding and signaling
*/
#include <ios>

#ifndef _HAS_CXX17
#error C++17 required
#endif

/*
http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-5.html

user defined error_category implements std::error_category
  and a "factory function" to deliver it as a singleton ref.
*/
inline const std::error_category & dbj_err_category()
{
	/*
	for an example look into <system_error> 
	implmentation of a class _System_error_category
	starting at line # 529
	*/
	struct dbj_err_category 
		// inheriting from MSVC STD implementation
		// has no meaning for dbj++ 
		// since dbj++ is not part of std
		: public std::error_category 
	{
		_NODISCARD virtual const char *name() const noexcept
		{
			static char name_[]{ "dbj_error_category" };
			return name_;
		}

		// error_code is looking here for the message
		// if we have inherited from std::error_code
		// and if we have caught std::system_exception 
		// and we have given this to its constructor
		// the system will call back here
		// when we call std::system_exception message()
		_NODISCARD virtual 
			std::string 
			message(int last_win32_err) const
		{
			switch (last_win32_err) {
			case 0: 
				return 
					dbj::err::message( ::dbj::err::errc::err_generic )
				.data() ;
			default: {
				// so the message is windows err message
				std::error_code ec =
					std::error_code(last_win32_err, std::system_category());
				return
					ec.message();
			}
			}
		}

		/* this is copy paste from MSVC STL 
		   <system_error> line # 562
		*/
		_NODISCARD virtual std::error_condition 
			default_error_condition(int _Errval) 
			  const noexcept override
		{	// make error_condition for error code (generic if possible)
			const int _Posv = std::_Winerror_map(_Errval);
			if (_Posv == 0)
			{
				return (std::error_condition(_Errval, std::system_category()));
			}
			else
			{
				return (std::error_condition(_Posv, std::generic_category()));
			}
		}
	};

	static dbj_err_category singleton_;
	return singleton_;
}

    // we can not place a friend inside std::error_code, so...
    using namespace dbj::console;
	inline void out
		(class std::error_code ec_)
	{
		::dbj::console::PRN.printf(
			"value:%d, category:'%s', message:'%s'",
			ec_.value(),
			ec_.category().name(),
			ec_.message().c_str()
		);
	}

/*
This is how msvc stl uses std::error_code, std::system_category and std::system_error

// C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726\crt\src\stl\xonce.cpp
// throw system_error containing GetLastError()
[[noreturn]] void _XGetLastError()
	{	
		error_code _Code(static_cast<int>(GetLastError()), system_category());
		throw(system_error(_Code));
	}

	NOTE: we do not care for the above since dbj++ is not part of a ::std
	nor is it system specific, unless we think of WIN32 as a system?
*/
	 

DBJ_TEST_SPACE_OPEN(how_to_use_system_error_portably)

#ifdef _MSC_VER
DBJ_TEST_UNIT(win32_system_specific_errors) {
	/*
	we can use ::std artefacts on MSVC platform to get system error messages
	*/
	char lpBuffer[64]{};
	// provoke system error
	DWORD DBJ_MAYBE(rv) = GetEnvironmentVariable(
		LPCTSTR("whatever_non_existent_env_var"),
		LPTSTR(lpBuffer),
		DWORD(64)
	);
	auto last_win32_err = ::GetLastError();
	// make error code from win32 err, int code 
	auto ec = std::error_code(last_win32_err, std::system_category());
	DBJ_TEST_ATOM(ec.value());
	DBJ_TEST_ATOM(ec.default_error_condition().message());
	DBJ_TEST_ATOM(ec.message());
	dbj::console::print("\n\n",ec, "\n");
}
#endif
DBJ_TEST_UNIT(standard) {

	using dbj::console::print;
	try
	{
// std::ercc is again ::std thing not dbj++
// int DBJ_MAYBE( err_enum ) = int(std::errc::protocol_error);
//
// we do *not* need to provide our own exception that will 
// inherit from std::system_error
	throw std::system_error
	( 
		// do we need to pass std::errc value here?
		// we could 'invent' a system where we start from some high value
		// e.g.1000, but that is obviously not platform agnostic
		int(std::errc::protocol_error), 
		// we could use dbj++ error category like here
		dbj_err_category(),	
		// this is arbitrary prompt
		" " __FILE__ " (" DBJ_EXPAND(__LINE__) ") \n"
	);
}
	catch (const std::system_error& ex)
	{

		// error_code is platform dependant
		std::error_code ec_ = ex.code();

		dbj::console::print(ec_);
		
		int							
				DBJ_MAYBE(ec_val_) = ec_.value();
		// std::error_category is ABC
		const std::error_category & 
				DBJ_MAYBE(ec_cat_) = ec_.category();
		auto	DBJ_MAYBE(cat_delivered_msg_) = ec_cat_.message(ec_val_);
		// error category is a singleton
		// error condition is platform independant
		std::error_condition		
				DBJ_MAYBE(ec_cond_) =  ec_.default_error_condition();
		std::string					
				DBJ_MAYBE(ec_msg_) = ec_.message();
		bool						
				DBJ_MAYBE(ec_not_zero) = ! ec_;

		print("error code: ", ex.code() , '\n' , '\n', "exception what: ", ex.what() , '\n' ) ;
	}
}

DBJ_TEST_SPACE_CLOSE


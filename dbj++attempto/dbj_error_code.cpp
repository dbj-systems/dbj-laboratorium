#include "pch.h"

/*
http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-5.html


namespace dbj::console {
	template<> inline void out<class std::error_code>
	(class std::error_code ec_)
	{
		::dbj::console::PRN.printf(
			"value:%d, category:'%s', message:'%s'",
			ec_.value(),
			ec_.category().name(),
			ec_.message().c_str()
		);
	}
}
*/
/*
   user defined error_category implements std::error_category
   and a "factory function" to deliver it as a singleton ref.
*/
const std::error_category & dbj_err_category()
{
	/*
	for an example look into <system_error> 
	implmentation of a class _System_error_category
	starting at line # 529
	*/
	struct dbj_err_category : public std::error_category 
	{
		_NODISCARD virtual const char *name() const noexcept
		{
			static char name_[]{ "dbj_error_category" };
			return name_;
		}

		// error_code is looking here for the message
		_NODISCARD virtual std::string message(int _Errval) const
		{
			switch (_Errval) {
			case 0: return "dbj default error message";
			case 1:
			case 2:
			case 3:
			default:
				return "dbj unknown error message";
			}
		}

		/* this is vopy paste from MSVC STL 
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
*/
	 

DBJ_TEST_SPACE_OPEN(how_to_use_system_error_portably)

DBJ_TEST_UNIT(portable) {
	std::error_code ec;
	// also does the set to 0
	auto DBJ_MAYBE(le) = dbj::win32::last_error();
	char lpBuffer[64]{};
#ifdef _WIN32
	DWORD DBJ_MAYBE(rv) = GetEnvironmentVariable(
		LPCTSTR("whatever"),
		LPTSTR(lpBuffer),
		DWORD(64)
	);
	ec = std::error_code(GetLastError(), std::system_category());
#else
	if (-1 == open(...))
		ec = std::error_code(errno, std::system_category());
#endif
	DBJ_TEST_ATOM(ec.message());

	dbj::console::print(ec);
}

DBJ_TEST_UNIT(standard) {

	using dbj::console::print;
#if 1
	try
	{
#endif
		int DBJ_MAYBE( err_enum ) = int(std::errc::protocol_error);

//		std::string throwerr;  
//		auto DBJ_MAYBE( c ) = throwerr.at(1); // throws std::system_error

		// we might provide our own exception that will 
		// inherit from std::system_error
	throw std::system_error( 
		err_enum, dbj_err_category(),	" " __FILE__ " (" DBJ_EXPAND(__LINE__) ") \n" 
	);
#if 1
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
#endif
}

DBJ_TEST_SPACE_CLOSE


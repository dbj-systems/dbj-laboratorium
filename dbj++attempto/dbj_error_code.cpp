#include "pch.h"

/*
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
	struct dbj_err_category : public std::error_category 
	{
		_NODISCARD virtual const char *name() const noexcept
		{
			static char name_[]{ "dbj errors category" };
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
	};

	static dbj_err_category singleton_;
	return singleton_;
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
	 

DBJ_TEST_SPACE_OPEN( dbj_error_code )

DBJ_TEST_UNIT(standard) {

	using dbj::console::print;
#if 0
	try
	{
#endif
		int DBJ_MAYBE( err_enum ) = int(std::errc::protocol_error);

//		std::string throwerr;  
//		auto DBJ_MAYBE( c ) = throwerr.at(1); // throws std::system_error

		// we might provide our own exception that will 
		// inherit from std::system_error
//		throw std::system_error( err_enum, dbj_err_category(),	" " __FILE__ " (" DBJ_EXPAND(__LINE__) ") \n" );
#if 0
}
	catch (const std::system_error& ex)
	{

		// error_code is platform dependant
		std::error_code ec_ = ex.code();
		
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


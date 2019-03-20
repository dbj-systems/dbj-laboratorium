#pragma once

/*
Following creates unique type (and instance)
as long as it is not repeated
somewehere else in the same app
-------------------------------------------------------------
void f1 () { printf("Start once!"); } ;
void f2 () { printf("End   once!"); } ;
static inline auto gbe =  dbj::entry_exit_singleton<f1,f2>() ;
*/

#pragma region entry_exit

namespace dbj {
	    
	    inline auto vvl = []() -> void {};
		using  voidvoidfun = void(*) ();
		using  vv_lambda = decltype( vvl );

		/*
		call ATSTART on start and at ATEND 
		on end of instance lifetime
		(possibly too)  simple class to do whatever two functions provided do when called
		*/
		template< typename BF, typename EF>
		class __declspec(novtable)	entry_exit final
		{
			BF begin_f_ ;
			EF end_f_   ;
			/*stop copying moving and swapping*/
			entry_exit(const entry_exit &) = delete;
			entry_exit & operator = (const entry_exit &) = delete;
			entry_exit(entry_exit &&) = delete;
			entry_exit && operator = (entry_exit &&) = delete;
			/* we also hide the default ctor so that instances of this class can not be delete-d */
			entry_exit() {};
		public:
			explicit entry_exit( BF begin_, EF end_) 
				: begin_f_(begin_), 
				  end_f_(end_)
			{ 
					// do something once and upon construction
					try {
						begin_f_();
					}
					catch (...) {
						throw "Calling ATSTART() failed in " __FUNCDNAME__;
					}
			}

			~ entry_exit() {
					// before destruction do something once
					try {
						end_f_();
					}
					catch (...) {
						// must not throw from destructor
						::dbj::core::trace("Calling ATEND() failed in %s", __FUNCSIG__);
					}
			}
		};

	} // dbj
#pragma endregion

/* standard suffix for every other header here */
#pragma comment( user, DBJ_BUILD_STAMP ) 

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
#pragma once
/*
in case you need/want COM init
but beware: there should be only one per app
and thread model must be always the same if
there is another one

#define DBJ_COMAUTOINIT
*/

#ifdef DBJ_COMAUTOINIT

#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
#endif
#define STRICT
#define NOSERVICE
// avoid min/max macros 
#define NOMINMAX
#include <windows.h>
#endif
// 
// ::CoInitialize() and the rest
#include <objbase.h>

namespace dbj::com {
	namespace internal {
		/*
		In internal namespace we hide the auto-initializer
		This ensures that COM is initialized �as soon as possible�
		This mechanism really works. Convince yourself once through the
		debugger, and then just forget about COM init/uninit.

		NOTE: this is slightly change to work and use 
		correctly the standard C++ 11/14/17
		*/
		class __declspec(novtable) COMAUTOINIT final
		{
			unsigned int & counter()
			{
				static unsigned int counter_ = 0;
				return counter_;
			}
			/*stop copying moving and swapping*/
			COMAUTOINIT & operator = (const COMAUTOINIT &) = delete;
			COMAUTOINIT(const COMAUTOINIT &&) = delete;
			COMAUTOINIT && operator = (const COMAUTOINIT &&) = delete;
			/* also singleton swap has no meaning */
			friend void swap(COMAUTOINIT &, COMAUTOINIT &) = delete;
			// {__noop;	}
			/*
			also hide the default ctor so that instances can not be delete-d

			If you call ::CoInitialize(NULL), after this method is used
			most likely the HRESULT will be :
			hRes = 0�80010106 � Cannot change thread mode after it is set.
			*/
			COMAUTOINIT() noexcept
			{
				const UINT & ctr = (counter())++;
				if (0 == ctr)
#if ( defined(_WIN32_DCOM)  || defined(_ATL_FREE_THREADED))
					HRESULT result = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
					HRESULT result = ::CoInitialize(NULL);
#endif
				/*TODO: log the result here*/
			}
		public:
			COMAUTOINIT(const COMAUTOINIT &) = default;

			/* NOTE: resilience in presence of multiple threads 
			   apparently this is it, in case compiler is C++11 or better
			*/
			static COMAUTOINIT & singleton()
			{
				static COMAUTOINIT  singleton_{};
				return singleton_;
			}

			~COMAUTOINIT()
			{
				const UINT ctr = --(counter());
				if (ctr < 1)
					::CoUninitialize();
			}

		};

		inline auto _comautoinit_single_instance_
			= COMAUTOINIT::singleton() ;

	} // anonspace
} // dbj::com


/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"

#endif

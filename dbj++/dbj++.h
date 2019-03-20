#pragma once

/* inclusion of this file defines the kind of a licence used */
#include "dbj_gpl_license.h"
#include "./win/dbj_win_inc.h"

#pragma warning( push )
#pragma warning( disable : 4244 )

#include "./core/dbj++core.h"
//
#include "./err/dbj_err.h"
//
#include "./util/dbj++util.h"
#include "./console/dbj++con.h"
#include "./testfwork/dbj_testing_interface.h"
#include "./numeric/dbj_numeric.h"
#include "./win/dbj++win.h"

#pragma warning( pop )

#if 0
/*
2018 MAY 31
Bellow is a solution for call once mechanism in MSVC
this is here as it was not certain how to achieve this using
the current version of MSCV compiler

- anonymous namespace implies default static linkage for variables inside
- worth repeating: this is safe in presence of multiple threads (MT) 
and is supported as a such by all major compilers
- inside is a lambda which is guaranteed to be called only once
- this pattern is also safe to use in header only situations
*/
namespace dbj_once {

	struct singleton final {};

	inline singleton & instance()
	{
		static singleton single_instance = []() -> singleton {
			// this is called only once
			// do some more complex initialization
			// here
#pragma message ("\compiling dbj_once 'call once' lambda ***************************************\n")
DBJ::TRACE("\nVisiting dbj_once 'call once' lambda ***************************************\n");
			return {};
		}();
		return single_instance;
	};

	inline singleton & singleton_middleton = instance();

 } // dbj_once
#endif
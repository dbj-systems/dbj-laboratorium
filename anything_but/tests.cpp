/*
	Type Handle to Avoid Implicit Conversions in standard C++

	Copyright(C) 2019 Dušan B. Jovanović (dbj@dbj.org)

	This program is free software : you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see < https://www.gnu.org/licenses/>.
*/

#include "dbj_nothing_but.h"
#include <iostream>

/*
 By "testing" in this context we basically mean: "does it or does it not compile"
 So we do not use any "testing framework"
 For compatibility: This is built in VS2017 15.9.X

 NOTE: Default warning level is "LEVEL 3" aka /W3

 Project settings are completely kept as deafult generated by Visual Studio
*/

extern "C" {
	void test_available_types();
	void test_creation();
	void test_assignments();
	void test_compatibility();
}

#pragma warning( push )
#pragma warning( disable : 4189 )
// warning C4189 : unused local variables

/*
-----------------------------------------------------------------------------
*/
extern "C" void test_available_types() {
	// fundamental types
	using just_bool = dbj::util::nothing_but<bool>;
}
/*
-----------------------------------------------------------------------------
*/
extern "C" void test_creation() {}
/*
-----------------------------------------------------------------------------
*/
extern "C" void test_assignments() {}
/*
-----------------------------------------------------------------------------
*/
extern "C" void test_compatibility() {}

#pragma warning( pop )

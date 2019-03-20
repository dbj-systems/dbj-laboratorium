#pragma once
#include "../dbj_gpl_license.h"

/*
include windows only from one place
and do it according to ancient windows lore
*/
#ifndef _INC_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define STRICT
#define NOSERVICE
#define NOMINMAX
#include <windows.h>
#endif

/*
Why was this?
#if (WINVER < NTDDI_WIN10_RS3)
#else
#error dbj++ requires Windows builds above REDSTONE 3 or above
#endif
*/

/*
rudimentary runtime version checks
https://docs.microsoft.com/en-us/windows/desktop/sysinfo/getting-the-system-version

#include <VersionHelpers.h>

*/



/*
GDI+
if required that is
note: the plan is to move any gdi+ related utils out of dbj++
*/
#ifdef DBJ_GDI_LINE
#ifndef _GDIPLUS_H
// because GDI+ 
// can not cope with
// NOMINMAX
#include <algorithm>
namespace Gdiplus
{
	using std::min;
	using std::max;
}
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")
#endif // _GDIPLUS_H
#endif



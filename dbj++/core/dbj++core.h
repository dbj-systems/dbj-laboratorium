#pragma once

#ifndef DBJ_CORE_INCLUDED
#define DBJ_CORE_INCLUDED

#include "../dbj_license.h"

/*
usage concept: use dbj++core always and only by including this header only
*/

// there is no point of avoiding windows include
// all of the dbj++ is firmly MSVC/WIN32 dependant
#include "dbj_win_inc.h"
#include "dbj_synchro.h"
#include "dbj_traits.h"
#include "dbj_runtime.h"
#include "dbj_insider.h"
#include "dbj_buffers.h"
#include "dbj_constexpr.h"
#include "dbj_utils_core.h"
#include "dbj_format.h"
#include "dbj_win32_core.h"

// _DBJ_META_CONVERTER_TESTING
#include "dbj_meta_converter.h"

#endif DBJ_CORE_INCLUDED

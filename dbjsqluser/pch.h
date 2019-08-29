#pragma once

#define DBJ_INCLUDE_STD_
#include <dbj++/dbj++required.h>

// we let dbj++ to use it's own wmain() 
// #define DBJ_WMAIN_USED
#include <dbj++/dbj++.h>

#include "../dbj++sql/dbj++sql.h"
#include "../dbj++ini/dbj++ini.h"

// here we use dbj syslog client 
// as we imagine this is some GUI app ...
#ifndef DBJ_SYSLOG
#define DBJ_SYSLOG (1==1)
#endif
#include <dbj++log/dbj++log.h>

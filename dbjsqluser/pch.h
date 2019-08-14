#pragma once
#ifndef PCH_H
#define PCH_H

 #include <sysinfoapi.h>
 #include <strsafe.h>
 #include <algorithm>
 #include <array>
 #include <cstddef>
 #include <cwctype>
 #include <cctype>
 #include <string>
 #include <vector>
 #include <map>
 #include <algorithm>

/*
I would like it is not, but since Aug 13, 2019 and after VS update it is
important what is included when.

I am actively watching that space to overcome this limitation
*/
#include <dbj++/core/dbj++core.h>
#include <dbj++/util/dbj++util.h>
#include <dbj++/console/dbj++con.h>
#include <dbj++/testfwork/dbj++testing_framework.h>
#define DBJ_SYSLOG (1==1)
#include <dbj++log/dbj++log.h>
#include <dbj++sql/test/core_tests.h>
#include <dbj++sql/test/dbj_easy_udf_sample.h>

#endif //PCH_H

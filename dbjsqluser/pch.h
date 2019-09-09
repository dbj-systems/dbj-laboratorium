#pragma once
#include <dbj--sqlite/dbj++sql.h>
// includes dbj_nanolib also
#include "dbj--nanolib/dbj++tu.h"

#if _WIN64
#pragma comment(lib, "dbj--sqlite3\\sqlite3.lib")
#else
#error SQLITE lib and dll are built for x64 applications.
#endif


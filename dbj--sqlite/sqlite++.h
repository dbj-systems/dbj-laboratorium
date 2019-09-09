#pragma once
/*
dbj.org -- 2019-01-26 -- created
wrap the sqlite into the namespace
obviously does *not* hide the #define's 
for example SQLITE_OK
so please learn what are they in sqlite3

2018-11-03	dbj@dbj.org
*/
#ifdef __cplusplus
namespace sqlite{
#endif

#include "sqlite3/sqlite3.h"

#ifdef __cplusplus
}
#endif

//#if _WIN64
//#pragma comment(lib, "sqlite3\\sqlite3.lib")
//#else
//#error SQLITE lib and dll are built for x64 applications.
//#endif

/*
final app depends on sqlite3.dll (x64) being available
*/

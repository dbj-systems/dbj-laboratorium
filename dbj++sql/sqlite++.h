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
/*
here we use the relative path
and sqlite3 static lib
for production code please download and install 
sqlite3 dll, put the header on the path
and include accordingly
*/
#include "../sqlite3_lib/sqlite3.h"

#ifdef __cplusplus
}
#endif

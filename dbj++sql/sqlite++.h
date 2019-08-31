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

if you hotly disagree and wish to use the dll
please download and install the sqlite3 dll, 
and include its header bellow
*/
#include "sqlite3/sqlite3.h"

#ifdef __cplusplus
}
#endif

#pragma comment(lib, "sqlite3.lib")

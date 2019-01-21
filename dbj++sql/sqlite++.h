#pragma once
/*

wrap the sqlite into the namespace
obviously does *not* hide the #define's 
for example SQLITE_OK
so please learn what are they in sqlite3

2018-11-03	dbj@dbj.org
*/
#ifdef __cplusplus
namespace sqlite{
#endif

#include "sqlite/sqlite3.h"

#ifdef __cplusplus
}
#endif

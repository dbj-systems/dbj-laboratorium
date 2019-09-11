/*
(c) 2019 by dbj@dbj.org -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/
*/  
// pch.cpp: source file corresponding to pre-compiled header; necessary for compilation to succeed


#include "pch.h"

#if _WIN64
#pragma comment(lib, "dbj--sqlite3\\sqlite3.lib")
#else
#error SQLITE lib and dll are built for x64 applications.
#endif

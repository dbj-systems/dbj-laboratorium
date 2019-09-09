// pch.cpp: source file corresponding to pre-compiled header; necessary for compilation to succeed

#include "pch.h"

#if _WIN64
#pragma comment(lib, "dbj--sqlite3\\sqlite3.lib")
#else
#error SQLITE lib and dll are built for x64 applications.
#endif

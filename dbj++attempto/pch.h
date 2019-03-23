#ifndef PCH_H
#define PCH_H

// #define _ENABLE_STL_INTERNAL_CHECK

#include <limits.h>

// dbj++ uses no std iostreams
// #include <iostream>

#include <fcntl.h>
#include <io.h>
#include <any>
#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#include <atomic>
#include <future>
#include <charconv>
#include <clocale>
#include <climits>
#include <variant>
#include <cstdlib>
#include <numeric>
#include <vector>
#include <random>

#define DBJ_LAMBDA_MATRIX_TESTING
#define DBJ_WMAIN_USED
#include <dbj++/dbj++.h>

#if 0
#ifdef STD
#error "STD already defined"
#else
#define STD ::std
#endif
#endif

#if ! _HAS_CXX17
#error C++17 required
#endif

#endif //PCH_H




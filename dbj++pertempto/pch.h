#pragma once
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
#include <system_error>
#include <string_view>
#include <clocale>
#include <variant>
#include <cstdlib>
#include <chrono>
#include <array>
#include <cassert>

#if ! _HAS_CXX17
#error C++17 required
#endif
#define DBJ_WMAIN_USED
#include <dbj++/dbj++.h>
// this whole project is about testing
// so ...
#include <dbj++/dbj++tests.h>




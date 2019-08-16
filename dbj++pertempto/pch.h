#pragma once
#define DBJ_INCLUDE_STD_
#ifdef DBJ_INCLUDE_STD_
#include <limits.h>
#include <fcntl.h>
#include <io.h>
#include <any>
#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#include <atomic>
#include <future>
#include <random>
#include <charconv>
#include <clocale>
#include <climits>
#include <variant>
#include <cstdlib>
#include <numeric>
#include <vector>
#include <array>
#include <cassert>

#include <cstdint>
#include <cstdio>
#include <optional>
#include <cassert>
#include <string>
#include <string_view>
#include <memory>
#endif

#define DBJ_WMAIN_USED
#include <dbj++/dbj++.h>
// this whole project is about testing
// so ...
#include <dbj++/dbj++tests.h>




#pragma once
#ifndef PCH_H
#define PCH_H

#include <limits.h>

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
#include <array>
#include <cassert>

#include <cstdint>
#include <cstdio>
#include <optional>
#include <cassert>
#include <string_view>
#include <memory>

// dbj++ 'bans' std iostreams
// #include <iostream>
#undef DBJ_WMAIN_USED
#include "../dbj++/dbj++.h"

#ifdef STD
#error "STD already defined"
#else
#define STD ::std
#endif


#if ! _HAS_CXX17
#error C++17 required
#endif

#endif //PCH_H

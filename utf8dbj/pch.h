
#ifndef PCH_H
#define PCH_H

#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <assert.h>
#include <math.h>
#include <fcntl.h>  
#include <io.h> 
#ifndef _HAS_CXX17
#	error C++17 please ...
#endif
#	define UNREF(...) (void)noexcept(__VA_ARGS__)
#define ST_2(x) #x
#define ST_1(x) ST_2(x)
#define ST(x) ST_1(x)
#define TU(x) printf("\n",ST(x));

#endif //PCH_H

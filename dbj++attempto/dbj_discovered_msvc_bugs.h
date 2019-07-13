#pragma once

namespace dbj::msvc_bug 
{
/*
https://developercommunity.visualstudio.com/content/problem/367032/overloading-for-native-array-of-ts-and-a-pointer-t.html
MSFT has recognized this and rectified in 
VS2019 16.3
https://godbolt.org/z/L_uhKR
*/

#include <string>
#include <stdio.h>

#define UN(x) (void)noexcept(x)

	template<typename T, size_t N>
	void work_on_t(const T(&arr)[N])
	{
		printf("\nwork_on_t ( const T(&arr)[N] )");
		UN(arr);
	}

	template<typename T>
	void work_on_t(const T* tp_)
	{
		printf("\nwork_on_t ( const T * tp_ )");
		UN(tp_);
	}

	int test()
	{
		int test_data[]{ 1, 2, 3 };
#ifdef _MSC_VER
		/*
		calling native array reference overload 
		for MSVC will compile only
		if there is a cast like here
		*/
		using intarr = int(&)[];
		work_on_t(intarr(test_data));
#else
		/*
		gnuc+ and clang are correctly calling an
		native array reference overload
		*/
		work_on_t(test_data);
#endif
		/*
		calling T* overload works for all 3 compilers
		*/
		work_on_t(std::begin(test_data));
	}
} // namespace dbj::msvc_bug

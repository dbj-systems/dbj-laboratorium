#pragma once
#include "pch.h"

/// <summary>
/// https://developercommunity.visualstudio.com/content/problem/173498/problem-with-if-constexpr.html?childToView=214529#comment-214529
/// </summary>

/// <summary>
/// CL -Bv reports 19.13.26128.0 after update to VS 15.6.2 
/// on 14MAR18
/// </summary>
namespace visual_studio_15_6_2::supports_api {

	using namespace std;

	namespace x {

		template<typename T>
		constexpr auto supportsAPI(T x)
			-> decltype(x, true_type{})
		{
			return {};
		}

		constexpr auto supportsAPI(...)
			-> false_type
		{
			return {};
		}


	}

	template<class T> 
	constexpr auto supportsAPI(T x) 
		-> decltype(x.begin(), x.end(), true_type{}) 
	{
		return {};
	}

	constexpr auto supportsAPI(...) 
		-> false_type 
	{
		return {};
	}

	template<class T> 
	void compute(T x) {
		if constexpr (supportsAPI(T{})) {
			// only gets compiled if the condition is true
			printf("API supported\n");
			x.begin();
			x.end();
		}
		else {
			printf("API not supported\n");
		}
	}

	struct Api1 {
		void begin() { }
		void end() { }
	};

	struct Api2 {
		void begin() { }
		// it is required by "compute()" 
		// but no Method2 here
	};

#ifdef DBJ_TESTING_ONAIR
	DBJ_TEST_UNIT(": Problem with 'if constexpr' ")
	{
		auto r1 = supportsAPI(string{});
		auto r2 = supportsAPI(Api2{});
		auto r3 = x::supportsAPI(& Api1::begin );
	}
#endif

}
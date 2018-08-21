#pragma once
#include "pch.h"

/// <summary>
/// CL -Bv reports 19.13.26128.0 after update to VS 15.6.2
/// </summary>

namespace CL_19_13_26128_0 {
#ifdef __clang__
	inline auto does_not_compile = [](auto _string)
		-> std::vector< decltype(_string) >
	{
		return std::vector<decltype(_string)>{_string};
	};
#endif
	inline auto does_compile = [](auto _string)
		//-> std::vector< decltype(_string) >
	{
		using string_type = decltype(_string);
		return std::vector<string_type>{_string};
	};

	inline auto if_constexpr = [](auto _string)
	{
		using string_type = decltype(_string);

		if constexpr (std::is_same<string_type, std::string>::value) {
			return std::vector<std::string>{_string};
		}
		else if constexpr (std::is_same<string_type, std::wstring>::value)
		{
			return std::vector<std::wstring>{_string};
		}
		else {
			return std::exception(
				"Exception in [ if_constexpr() ] argument has to be std::string or std::wstring"
			);
		}
	};
}

#if 0
namespace CL_19_12_25835_0 {

	inline auto does_not_compile = [](auto _string)
		-> std::vector< decltype(_string) >
	{
		return std::vector<decltype(_string)>{};
	};
	inline auto does_compile = [](auto _string)
		//-> std::vector< decltype(_string) >
	{
		return std::vector<decltype(_string)>{};
	};

	inline auto if_constexpr = [](auto _string)
	{
		using string_type = decltype(_string);

		if constexpr (std::is_same<string_type, std::string>::value) {
			return std::vector<std::string>{};
		}
		else if constexpr (std::is_same<string_type, std::wstring>::value)
		{
			return std::vector<std::wstring>{};
		}
		else {
			return std::exception(
		"Exception in [ if_constexpr() ] argument has to be std::string or std::wstring"
			);
		}
	};
}
#endif

#ifdef DBJ_TESTING_ONAIR

namespace {
	using namespace std;

	DBJ_TEST_UNIT(": lambda_runtime_retval_test ")
	{
		auto test_ = []() -> bool {

			string &&		narrow_{ "NARROW" };
			wstring &&		wide_{ L"WIDE" };

			if (__argv)
				auto v1 = CL_19_13_26128_0::does_compile(*__argv);

			if (__wargv)
				auto v2 = CL_19_13_26128_0::does_compile(*__wargv);

#ifdef __clang__
			auto v3 = CL_19_13_26128_0::does_not_compile(narrow_);
			auto v4 = CL_19_13_26128_0::does_not_compile(wide_);
#endif
			auto v5 = CL_19_13_26128_0::if_constexpr(narrow_);
			auto v6 = CL_19_13_26128_0::if_constexpr(wide_);

			return 1 == 1;
		};

		auto just_do_it_here_and_now = test_();
	}
}
#endif
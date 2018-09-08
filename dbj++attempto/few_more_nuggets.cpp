﻿// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include <clocale>

namespace dbj::samples { // beware of anonymous namespace

	using namespace std; // beware
/***********************************************************************************/
	using leader_name_type = const wchar_t *;
	leader_name_type leaders[]{ L"Ленин", L"Сталин", L"Маленков",
L"Хрущёв", L"Брежнев", L"Андропов", L"Черненко", L"Горбачёв", L"안녕하세요", L"안녕히 가십시오" };

	/***********************************************************************************/
	template<
		typename T, size_t N,
		typename tuple_type = ::dbj::tt::array_as_tuple_t<T, N>
	>
		tuple_type sequence_to_tuple(const T(&narf)[N])
	{
		return ::dbj::arr::native_arr_to_tuple(narf);
	}

	template<
		typename T, size_t N,
		typename arr_type = ::std::array<T, N>
	>
		inline arr_type sequence_to_std_array(const T(&narf)[N])
	{
		arr_type std_arr;
		std::copy(narf, narf + 5, std_arr.data());
		return std_arr;
	}

	DBJ_TEST_UNIT(tuple_production)
	{
		int iarr[] = { 1,2,3,4,5,6,7,8 };
		auto t1 = sequence_to_tuple({ 1,2,3,4,5 });
		auto t2 = sequence_to_tuple(iarr);
	}

	/***********************************************************************************/
	/* generic lambda solution */
	auto summa = [](auto first, auto ... second) {

		if constexpr (sizeof ... (second) > 0) {
			return first + summa(second ...);
		}
		else {
			return first;
		}
	};

		auto lambda_holder = [](auto invocable_arg)
		{
			return [=]() {
				return invocable_arg;
			};
		};

		auto zbir = lambda_holder(summa);

		struct apply_helper final
		{
			// apply the pair
			template< typename T1, typename T2>
			auto operator () (T1 v1, T2 v2) {
				return  apply(zbir(), make_pair(v1, v2));
			}
			// apply the tuple or args
			template< typename ... ARGS >
			auto operator ()  (std::tuple<ARGS ...> tuple_) {
				return  apply(zbir(), tuple_);
			}

			// apply the native array 
			template< typename T, size_t N>
			auto operator () (const T(&array_)[N]) {
				array<T, N> std_array = dbj::arr::native_to_std_array(array_);
				return  apply(zbir(), std_array);
			}
		};

	/***********************************************************************************/
	DBJ_TEST_UNIT(a_lot_of_nuggets)
	{
		auto prev_fn = dbj::console::get_font_name();

		// begin() can throw the exception
		// end() is guaranteed, even in the presence of exceptions
		const auto & begin_end_ = dbj::begin_end(
			[&]() {
			std::sort(begin(leaders), end(leaders), [](auto strA, auto strB) {
				return std::wcscmp(strA, strB) < 0;
			});
			char* a = std::setlocale(LC_ALL, "");
			dbj::console::set_extended_chars_font();
		},
			[&]() {
			dbj::console::set_font(prev_fn);
		}
		);

		using leader_name_type_string
			= std::basic_string< dbj::tt::to_base_t<leader_name_type>  >;
		leader_name_type_string zbir;

		// this happens between begin() and end()
		for (const wchar_t * leader : leaders) {
			dbj::console::print(leader, '\n');
			zbir = (summa(zbir, leader_name_type_string(leader)));
		}
		{
			int ai[]{ 1,2,3,4,5 };
			apply_helper aplikator;
			auto r0 = aplikator(1, 2);
			auto r1 = aplikator(make_tuple(1, 2, 3, 4, 5));
			auto ili = { 1, 2, 3, 4, 5 };
			auto r2 = aplikator({ 1,2,3 });
			auto r3 = aplikator(ai);
			auto unused[[maybe_unused]] = 42;
		}

		DBJ_TEST_ATOM(std::apply(summa, std::make_pair(1, 2)));
		DBJ_TEST_ATOM(std::apply(summa, std::make_tuple(2.0f, 3.0f, 1, 2)));
		DBJ_TEST_ATOM(std::apply(summa, std::make_tuple(2.0f, 3.0f,
			std::apply(summa, std::make_pair(11, 12))
		)));
	}
} // anon ns
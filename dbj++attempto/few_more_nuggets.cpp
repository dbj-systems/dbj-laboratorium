// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"

namespace dbj::samples { // beware of anonymous namespace

	using namespace std; // beware
/***********************************************************************************/
	using leader_name_type = const wchar_t *;
	
	using leader_name_type_string
		= std::basic_string< dbj::tt::to_base_t<leader_name_type>  >;

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

		using lambda_holder_type = 
			invoke_result_t < decltype( lambda_holder ), decltype(summa) > ;

		template<typename INVOCABLE >
		struct apply_helper final
		{
			const INVOCABLE & invocable_; 

			// apply the pair
			template< typename T1, typename T2>
			auto operator () (T1 v1, T2 v2) {
				return  apply(invocable_, make_pair(v1, v2));
			}
			// apply the tuple or args
			template< typename ... ARGS >
			auto operator ()  (std::tuple<ARGS ...> tuple_) {
				return  apply(invocable_, tuple_);
			}

			// apply the native array 
			template< typename T, size_t N>
			auto operator () (const T(&array_)[N]) {
				array<T, N> std_array = dbj::arr::native_to_std_array(array_);
				return  apply(invocable_, std_array);
			}
		};


		auto make_apply_helper = [](auto lambda_) {
			return apply_helper<decltype(lambda_)>{lambda_};
		};

	/***********************************************************************************/
	DBJ_TEST_UNIT(a_lot_of_nuggets)
	{
		auto prev_fn = dbj::console::get_font_name();

		// begin() can throw the exception
		// end() is guaranteed, even in the presence of exceptions
		const auto & begin_end_ = ::dbj::entry_exit(
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

		leader_name_type_string zbir;

		// this happens between begin() and end()
		for (const wchar_t * leader : leaders) {
			dbj::console::print(leader, '\n');
			zbir = (summa(zbir, leader_name_type_string(leader)));
		}
		{
			int ai[]{ 1,2,3,4,5 };


			auto aplikator = make_apply_helper(summa);

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

		auto buf = dbj::str::optimal_buffer<char>();

#define UNUSED [[maybe_unused]] 

		UNUSED auto [ ptr , erc ]  = std::to_chars(buf.data(), buf.data() + buf.size(), 42);

		auto rez = 0 == dbj_ordinal_string_compareA(buf.data(), "42", true);

	}
#pragma region https://stackoverflow.com/questions/52244640/if-constexpr-and-c4702-and-c4100-and-c4715/52244957#52244957
	
#ifdef _DEBUG
#define DBJ_NOT_USED(x) void{x}
#else
#define DBJ_NOT_USED(x) 
#endif

	/***********************************************************************************/
	DBJ_TEST_UNIT(a_lot_of_variations)
	{
		using specimen_type = dbj::tt::fundamental_twins<int, const int &>;
		specimen_type ft;
		specimen_type::decay_1 v1{};
		specimen_type::decay_2 v2{};
		specimen_type::type v3{};
		specimen_type::value_type v4{};
		auto wot = ft();

		int a = 13;
		const int & b = 42;
		auto are_they = dbj::tt::same_fundamental_types(a, b);
	}
#pragma endregion
} // anon ns
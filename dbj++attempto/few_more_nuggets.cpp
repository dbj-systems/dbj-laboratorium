#include "pch.h"

#include "types_and_structures/mixing_lambada.h"
#include "algorithms/dbj_vector_element_to_index.h"
#include "algorithms/dbj_apply.h"

namespace dbj::samples { // beware of anonymous namespace

	using namespace std; // beware
/***********************************************************************************/
	using leader_name_type = const wchar_t *;

	using leader_name_type_string
		= std::basic_string< ::dbj::tt::to_base_t<leader_name_type>  >;

	leader_name_type leaders[]{ L"Ленин", L"Сталин", L"Маленков",
L"Хрущёв", L"Брежнев", L"Андропов", L"Черненко", L"Горбачёв", L"안녕하세요", L"안녕히 가십시오" };

/*
	C++ dynamic arrays .. also here https://en.cppreference.com/w/cpp/language/new
	int n = 42; 
	constexpr int m = 13;
	double a[n][5]; // error
	auto p1 = new double[n][m]; // okay
	auto p2 = new double[5][n]; // error
	
	how to delete 2d array properly in C++
	void delete_(T **M) {
		delete[] M[0];
		delete[] M;
	};
*/

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
		std::copy(narf, narf + N, std_arr.data());
		return std_arr;
	}

	/*
	Transform initializer list into std::vector
	*/
	template<typename T, typename outype = std::vector<T> >
	inline
		constexpr outype
		inil_to_vector(std::initializer_list<T> inil_)
	{
		return { inil_.begin(), inil_.end() };
	}
	/*******************************************************************************/
	template<typename T>
	constexpr size_t get_extent(const T * tar)
	{
		if constexpr (std::is_array_v<T>) {
			constexpr size_t rank_ = std::rank_v<T>;
			return std::extent_v< std::remove_pointer_t<T>, rank_ - 1>;
		}
		else {
			return {};
		};
	}
	/*******************************************************************************/
	template< typename T, size_t N>
	inline
		std::array<T, N>
		v_2_a(const std::vector<T> & vector_) {

		T(&narf)[N] =
			*(T(*)[N])vector_.data();

		std::array<T, N> retval_;

		std::copy(narf, narf + N, retval_.data());

		return retval_;
	};

	/*******************************************************************************/
	DBJ_TEST_UNIT(tuple_production)
	{
		int iarr[] = { 1,2,3,4,5,6,7,8 };
		auto t1 = sequence_to_tuple({ 1,2,3,4,5 });
		auto t2 = sequence_to_tuple(iarr);

		auto inil_ = { 1,2,3,4,5 };

		auto rezult = inil_to_vector(inil_);

		using v_type = decltype(rezult);
		using v_value_type = typename v_type::value_type;

		using data_arr_type = v_value_type[];

		auto & DBJ_MAYBE(refintarr) = *rezult.data();

		// const size_t extent_ = get_extent(refintarr);
		{
			std::vector<int> vint{ 1,2,3 };
			using inarr_type = int[];
			auto & DBJ_MAYBE(inarr) = *vint.data();
		}

		{

			using C3 = ::dbj::arr::ARH<char, 3>;

			C3::ARV v_of_c{ 'A','B','C' };

			C3::ARR  DBJ_MAYBE(std_arr) = C3::from_vector(v_of_c);

			{  // manual way
				constexpr size_t N = 3;
				auto DBJ_MAYBE(std_arr_) = v_2_a<char, N>(v_of_c);
			}

			const char(&DBJ_MAYBE(ref_to_arr_of_chars))[3] =
				*(char(*)[3])v_of_c.data();

			const char(&DBJ_MAYBE(ref_to_arr_of_chars_2))[] =
				*(char(*)[])v_of_c.data();

			const size_t N = v_of_c.size();
			char * vla_ = (char*)::std::calloc(N, sizeof(char));

			std::copy(v_of_c.data(), v_of_c.data() + v_of_c.size(), vla_);

			auto DBJ_MAYBE(isit) = std::is_array_v< decltype(vla_)>;

			::std::free(vla_);

			auto DBJ_MAYBE(dummy) = true;
		}
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

	template<typename T>
	using lambda_holder_type =
		invoke_result_t < decltype(lambda_holder), T >;


	/***********************************************************************************/
	DBJ_TEST_UNIT(a_lot_of_nuggets)
	{
		using namespace std;
		wstring prev_fn = ::dbj::console::get_font_name();

		// begin() can throw the exception
		// end() is guaranteed, even in the presence of exceptions
		const auto & DBJ_MAYBE(begin_end_) = ::dbj::entry_exit(
			[&]() {
			std::sort(begin(leaders), end(leaders), [](auto strA, auto strB) {
				return std::wcscmp(strA, strB) < 0;
			});
			char* DBJ_MAYBE(a) = std::setlocale(LC_ALL, "");
			::dbj::console::set_extended_chars_font();
		},
			[&]() {
			::dbj::console::set_font(prev_fn.c_str());
		}
		);

		leader_name_type_string zbir;

		// this happens between begin() and end()
		for (auto leader : leaders) {
			::dbj::console::print(leader, '\n');
			zbir = (summa(zbir, leader_name_type_string(leader)));
		}
		// applicator helper testing
		{
			int ai[]{ 1,2,3,4,5 };
			auto aplikator = make_apply_helper(summa);

			DBJ_TEST_ATOM(aplikator(1, 2));
			DBJ_TEST_ATOM(aplikator(make_tuple(1, 2, 3, 4, 5)));
			auto ail = { 1, 2, 3, 4, 5 };
			DBJ_TEST_ATOM(aplikator({ 1,2,3 }));
			DBJ_TEST_ATOM(aplikator(ai));
		}

		DBJ_TEST_ATOM(std::apply(summa, std::make_pair(1, 2)));
		DBJ_TEST_ATOM(std::apply(summa, std::make_tuple(2.0f, 3.0f, 1, 2)));
		DBJ_TEST_ATOM(std::apply(summa, std::make_tuple(2.0f, 3.0f,
			std::apply(summa, std::make_pair(11, 12))
		)));

		auto buf = ::dbj::str::optimal_buffer<char>();

		[[maybe_unused]]  auto[ptr, erc] = std::to_chars(buf.data(), buf.data() + buf.size(), LONG_MAX);

		DBJ_TEST_ATOM(dbj_ordinal_string_compareA(buf.data(), "42", true));

	}
#pragma region https://stackoverflow.com/questions/52244640/if-constexpr-and-c4702-and-c4100-and-c4715/52244957#52244957

	/***********************************************************************************/
	DBJ_TEST_UNIT(a_lot_of_variations)
	{
		using twins_ = ::dbj::tt::fundamental_twins<int, const int &>;
		twins_ ft;
		twins_::decay_1 DBJ_MAYBE(v1) {};
		twins_::decay_2 DBJ_MAYBE(v2) {};
		twins_::type DBJ_MAYBE(v3) {};
		twins_::value_type DBJ_MAYBE(v4) {};
		auto DBJ_MAYBE(wot) = ft();

		int DBJ_MAYBE(a) = 13;
		const int & b = 42;
		auto DBJ_MAYBE(are_they) = dbj::tt::same_fundamental_types(a, b);
		DBJ_NOUSE(dbj::tt::same_fundamental_types(a, b));
		DBJ_NOUSE(a / b);
	}
#pragma endregion
	} // namespace dbj::samples 





/*
	In any case, std::terminate calls the currently installed
	std::terminate_handler.The default std::terminate_handler
	calls std::abort.

	To use user defined std::terminate_handler, the C++ implementation
	provides a default std::terminate_handler.
	If the null pointer value is installed (by means of std::set_terminate),
	the implementation may restore the default handler instead.

	int main()
		{
	std::set_terminate([](){ std::cout << "Unhandled exception\n"; std::abort();});
	throw 1;
		}
	*/

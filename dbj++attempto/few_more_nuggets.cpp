// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"

#include "../dbj++clib/dbjclib.h"

// #include <experimental/filesystem>

namespace dbj::samples { // beware of anonymous namespace

	using namespace std; // beware
/***********************************************************************************/
	using leader_name_type = const wchar_t *;

	using leader_name_type_string
		= std::basic_string< dbj::tt::to_base_t<leader_name_type>  >;

	leader_name_type leaders[]{ L"Ленин", L"Сталин", L"Маленков",
L"Хрущёв", L"Брежнев", L"Андропов", L"Черненко", L"Горбачёв", L"안녕하세요", L"안녕히 가십시오" };

	/***********************************************************************************/
	// C++ dynamic arrays .. also here https://en.cppreference.com/w/cpp/language/new
	// int n = 42; 
	// constexpr int m = 13;
	// double a[n][5]; // error
	// auto p1 = new double[n][m]; // okay
	// auto p2 = new double[5][n]; // error


	/* static matrix type */
	template <
		typename	T,
		size_t		R,
		size_t		C,
		size_t		MAX = INT_MAX /*0x7FFFFFFF -- limits.h*/
	>
		class dbj_field final {
		static_assert(
			(R * C * sizeof(T)) <= MAX,
			"Total size of dbj_field must not exceed 0x7FFFFFFF bytes"
			);
		/*
		The 64-bit PECOFF executable format used on Windows
		doesn't support creating executables that have
		a load size of greater than 2GB
		*/
		public:
			using type = T[R][C];
			using reference_type = T(&)[R][C];
			static constexpr size_t rows() noexcept { return R; };
			static constexpr size_t cols() noexcept { return C; };
			static constexpr size_t size() noexcept { return R * C * sizeof(T); };
			static constexpr size_t max_size() noexcept { return MAX; };

			dbj_field() { }
			~dbj_field() { }

			/*
			Not returning pointers but references.
			dbj_field<int,6,6> fld ;
			auto ref_ = fld.data() ;
			 ref type is:
			dbj_field<int,6,6>::reference_type
			*/
			reference_type data() noexcept { return this->data_; }
			reference_type data() const noexcept { return this->data_; }

			// set and get for non const instances
			T & operator () (size_t r, size_t c) noexcept { return data_[r][c]; }
			// no set on const instances
			T operator () (size_t r, size_t c) const noexcept { return data_[r][c]; }

		private:
			mutable type data_{};
			/*
			how to delete 2d array properly in C++
			void delete_(T **M) {
				delete[] M[0];
				delete[] M;
			};
			*/
	};

	DBJ_TEST_UNIT(cpp_dynamic_arrays) {
		/* */
				auto test_ = []( const int & arg_ )
				{
					if ( nullptr == &arg_ ) return ;
					 int DBJ_MAYBE(x) = arg_ ;
				};
				int *ip_ = nullptr;
				test_(0);			// OK
				test_(*ip_);		// dbj has caught it
		/*	*/


		using dbj_matrix = dbj_field<int, 0xFF, 0xFF>;
		const dbj_matrix d2d;

		const auto DBJ_MAYBE(d1) = d2d.data();
		// returns ref to internal matrix
		// with sizes thus reachable
		const auto & dr = d2d.data();
		auto DBJ_MAYBE(rank_) = std::rank_v< std::remove_reference_t<std::remove_cv_t<decltype(dr)>>    >;
		auto DBJ_MAYBE(sz_1) = std::extent_v< std::remove_reference_t<std::remove_cv_t<decltype(dr)>>, 0 >;
		auto DBJ_MAYBE(sz_2) = std::extent_v< std::remove_reference_t<std::remove_cv_t<decltype(dr)>>, 1 >;

		// const instance: can get but not set 
		auto DBJ_MAYBE(r) = d2d(4, 4); //  = 42; --- error

		auto DBJ_MAYBE(size_in_byes_) = d2d.size();
	}
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
			using DBJ_MAYBE(inarr_type) = int[];
			auto & DBJ_MAYBE(inarr) = *vint.data();
		}

		{

			using C3 = dbj::arr::ARH<char, 3>;

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

			STD free(vla_);

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

	template<typename INVOCABLE >
	struct apply_helper final
	{
		const INVOCABLE & invocable_;

		// apply the pair of values
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
		// also takes care of init list call
		// which will otherwise clash with var args 
		// overload above
		template< typename T, size_t N>
		auto operator () (const T(&array_)[N]) {
			array<T, N> std_array = dbj::arr::native_to_std_array(array_);
			return  apply(invocable_, std_array);
		}
	};


	auto make_apply_helper = [](auto invocable_) {
		return apply_helper<decltype(invocable_)>{invocable_};
	};

	/***********************************************************************************/
	DBJ_TEST_UNIT(a_lot_of_nuggets)
	{
		auto prev_fn = dbj::console::get_font_name();

		// begin() can throw the exception
		// end() is guaranteed, even in the presence of exceptions
		const auto & DBJ_MAYBE(begin_end_) = ::dbj::entry_exit(
			[&]() {
			std::sort(begin(leaders), end(leaders), [](auto strA, auto strB) {
				return std::wcscmp(strA, strB) < 0;
			});
			char* DBJ_MAYBE(a) = std::setlocale(LC_ALL, "");
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

		auto buf = dbj::str::optimal_buffer<char>();

		[[maybe_unused]]  auto[ptr, erc] = std::to_chars(buf.data(), buf.data() + buf.size(), 42);

		auto DBJ_MAYBE(rez) = 0 == dbj_ordinal_string_compareA(buf.data(), "42", true);

	}
#pragma region https://stackoverflow.com/questions/52244640/if-constexpr-and-c4702-and-c4100-and-c4715/52244957#52244957

	/***********************************************************************************/
	DBJ_TEST_UNIT(a_lot_of_variations)
	{
		using twins_ = dbj::tt::fundamental_twins<int, const int &>;
		twins_ ft;
		twins_::decay_1 DBJ_MAYBE(v1) {};
		twins_::decay_2 DBJ_MAYBE(v2) {};
		twins_::type DBJ_MAYBE(v3) {};
		twins_::value_type DBJ_MAYBE(v4) {};
		auto DBJ_MAYBE(wot) = ft();

		int DBJ_MAYBE(a) = 13;
		const int & b = 42;
		auto DBJ_MAYBE(are_they) = dbj::tt::same_fundamental_types(a, b);
		DBJ_VANISH(dbj::tt::same_fundamental_types(a, b));
		DBJ_VANISH(a / b);
	}
#pragma endregion

	DBJ_TEST_UNIT(clang_and_dbj)
	{
		// reaching to C code
		using dbj::clib::location_;
		using dbj::clib::location_descriptor;

		location_descriptor *loc_desc_0, *loc_desc_2, *loc_desc_3;
		// begin block
		{
			dbj::entry_exit{
				[&]() {
				// take one
				loc_desc_0 = location_.create(__LINE__, __FILE__);
				},
				[&]() {
					// take two
					loc_desc_2 =
						location_.create(__LINE__, __FILE__);

					// release the 2 but leave the 1
					location_.release(&loc_desc_2);

					// take three, should reuse the second registry slot
					// thus loc_desc_2 == loc_desc_3
					loc_desc_3 =
						location_.create(__LINE__, __FILE__);
					}
			};
		}

		auto ld0 = *loc_desc_0;
		auto ld3 = *loc_desc_3;

		auto DBJ_MAYBE(dummy) = true;
	}
} // anon ns
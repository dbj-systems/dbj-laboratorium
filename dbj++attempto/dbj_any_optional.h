#pragma once
#include "pch.h"
#pragma region ANY WRAPPER
namespace dbj {

	namespace any {

		using namespace std;
		template <typename T> class wrapper;

		template <typename T>
		class wrapper final
		{
		public:
			using type = wrapper;
			// types
			// typedef wrapper any_wrapper_type;
			typedef std::remove_cv_t<T> value_type;

		private:
			static_assert(!std::is_reference_v< std::remove_cv_t<T> >,
				"[dbj::wrapper] Can not hold a reference type");

			static_assert(!std::is_array_v<std::remove_cv_t<T> >,
				"[dbj::wrapper] Can not hold an array type");

			std::any any_;

			// an example how to overload dbj::console::out for UDT
			friend void out(const type & anyt_)
			{
				::dbj::console::out(anyt_.get());
			}

		public:

			// we take references, pointers and a such ;)
			explicit wrapper(const value_type & ref) noexcept : any_(ref) {}
			// we take no r-references or values
			// wrapper(value_type & ref) noexcept = delete; // : any_(move(ref)) {	}

			wrapper() noexcept {	}

			// only if function is stored
			template< class... ArgTypes	>
				auto /* invoke_result_t< value_type &, ArgTypes...> */
				operator() (ArgTypes&&... args) const 
			{
				static_assert(is_invocable_v<value_type, ArgTypes ... >
					, "[dbj any wrapper] does not hold invokable type ");

				if (!empty()) 
				{
					value_type fun = get();
					return invoke(fun, forward<ArgTypes>(args)...);
				}
				else {
					throw dbj::exception(" can not call on empty data wrapped ");
				}
			}

			constexpr value_type get() const {
				try {
					return any_cast<value_type>(this->any_);
				}
				catch (std::bad_any_cast & x) {
					throw dbj::exception(x.what());
				}
			}

			constexpr bool empty() const noexcept {
				return !(this->any_).has_value();
			}

		}; // wrapper

		// factory methods ----------------------------------------

		template < typename T >
			static wrapper<T> make(T val_)
		{
			// can make only from values
			//  see the ctor deleted
			return wrapper< T >(val_);
		};

		// template < typename T >	inline auto make(T && val_) = delete ;

		// given T(&)[N]
		// returns std::array of N * wrapper<T>
		// thus array of any's effectively
		// made from native array
		template <
			typename T,	std::size_t N,	typename ANYW = wrapper<T>
		>
		static std::array< ANYW, N > 
			range(const T(&arrf)[N])
		{
			std::array< ANYW, N > rezult;
			std::size_t j = 0;

			for (auto && element : arrf) {
				rezult[j] = any::make(element);
				j += 1;
			}
			return rezult;
		};

		// r-references to constants/literlas <-- no can do
		template <
			typename T,
			std::size_t N,
			typename ANYW = wrapper<T>,
			typename RETT = std::array< ANYW, N >,
			typename std_arr_t = RETT
		>
			static RETT range(const T(&&arrf)[N]) = delete;
} // any

} // dbj
#pragma endregion

DBJ_TEST_SPACE_OPEN(dbj_any_wrapper_testing)

	// TESTING
	struct X;
	extern const X empty_x;
	struct X final
	{
		static inline const X & ref = empty_x;
		operator const char * () const noexcept { return "X test struct, instance"; }
	};
	inline const X empty_x;


	DBJ_TEST_UNIT(dbj_strong_optional)
	{
			using int_		= dbj::optional_handle<int>;
			using float_	= dbj::optional_handle<float>;
			using size_		= dbj::optional_handle<unsigned int>;
			// interesting
			using void_		= dbj::optional_handle<void>;
			int_	ii		= 42;
			float_	ff		= 42.0f;
			size_	ss		= 1024U;

			int_			i2{ 84 };
			float_			f2{ 84.0f };
			size_			 s2{ 2048U };

			DBJ_TEST_ATOM( ii = i2 );
			DBJ_TEST_ATOM( ff = f2 );
			DBJ_TEST_ATOM( ss = s2 );
	}

		DBJ_TEST_UNIT(dbj_any_wrapper)
	{
		using namespace ::dbj::any;
			int int_arr[]{ 1,2,3 };
			auto any_0 = DBJ_TEST_ATOM(range(int_arr));
				auto not_a_temporary = "YES CAN DO"s ;
			auto any_2 = DBJ_TEST_ATOM(make(not_a_temporary));
			auto DBJ_MAYBE(any_3) = DBJ_TEST_ATOM(any_2); // copy wrapper to wrapper
			auto any_4 = make([&](auto x) { return typeid(x).name();  });
			auto DBJ_MAYBE(rez_4) = any_4(true);
	}

	DBJ_TEST_SPACE_CLOSE
/*
Copyright 2017 by dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

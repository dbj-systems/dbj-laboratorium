#pragma once
#include "pch.h"
#include <optional>

namespace dbj::samples::optnl {

	using namespace std;
	/*
	Idea while reading: https://blogs.msdn.microsoft.com/vcblog/2018/09/04/stdoptional-how-when-and-why/

	and https://en.wikipedia.org/wiki/Option_type

	  idea: if S is a template, with ctor/dtor one could do

	  S<int>    int_(42);
	  S<bool>   bool_(true) ;
	  S<string> string_("Hola!") ;

	  These are very complete objectified intrinsics?
	  Also, are these (a holy grail) Strong Typedefs, (almost) for free?

	  using int_ = S<int> ; // a distinct type

	*/

	template<typename T>
	struct strong final
	{
		/* here we can put demanding static asserts */
		using value_type = T;

	// must have some *non default*
	// initial value
	// MSVC says (2018 Oct) :
	// error C4700: uninitialized local variable used
	// using dbj strong<> that can not happen
	// no default ctor stops creation of T 
	// with default intrinsic values
		strong() = delete;

		strong(T arg) {
			value_.emplace(arg);
		}

		operator T& () noexcept {
			// _ASSERTE( value_ );	return *value_;
			// but, I prefer an exception when value_ is not initialized:
			return value_.value();
		}
	private:
		std::optional<T> value_;
	};

	//
	struct X;
	extern const X empty_x;
	struct X final
	{
		static inline const X & ref = empty_x ;
		operator const char * () const noexcept { return "X test struct, instance"; }
	};
	inline const X empty_x;

	template<typename T, size_t N>
	using ref_to_arr = std::reference_wrapper<const T[N]>;

	template<typename T, size_t N>
	using ptr_to_arr = std::shared_ptr<T[N]>;

	DBJ_TEST_UNIT(dbj_strong_optional)
	{
		// native array on the stack
		// the point of creation
		int int3[]{1,2,3};
		// creation of the reference object
		// to the native stack created array
		ref_to_arr<int, 3> i3 = std::cref(int3) ;
		// get to the reference of the native array contained
		const ref_to_arr<int, 3>::type & i3ref = i3.get();
		
		auto DBJ_MAYBE( well_is_it ) = dbj::tt::is_array_(i3ref);

		// smart pointer is not a solution, why?
		ptr_to_arr<int,3> DBJ_MAYBE( spi3) ( new int[3]{ 1,2,3 } );
		// OK
		_ASSERTE( 2 == spi3[1] );

		// if declared to hold int[]
		// element type is int
		// if declared to hold int
		// element type is int
		ptr_to_arr<int, 3>::element_type * arr_p = spi3.get() ;

		// but we 'know' it is an array, so ...
		bool DBJ_MAYBE( is_it_then ) = dbj::tt::is_array_((int( & ) [3])(arr_p));


#pragma region strong intrisics
		{
			// usual "striong types" mantra:
			// "not just" int, float, size_t etc ...
			// but with some behaviour added
			// dbj micro strong types are this:
			using int_ = strong<int>;
			using float_ = strong<float>;
			using size_ = strong<unsigned int>;
			// interesting
			using void_ = strong<void>;
			// howto instantiate?
			// void_ v_ = { (void)0 };

			// var must have some non default
			// initial value
			// otherwise standard C++ won't compile
			// for example, MSVC says:
			// error C4700: uninitialized local variable used
			// using dbj strong<> that can not happen

			int_	ii = 42;
			float_	ff = 42.0f;
			size_	ss = 1024U;

			int_    i2{ 84 };
			float_    f2{ 84.0f };
			size_    s2{ 2048U };

			ii = i2;
			ff = f2;
			ss = s2;

			dbj::console::print(
				"\n", (int_::value_type)ii,
				"\t", (float_::value_type)ff,
				"\t", (size_::value_type)ss);
		}
#pragma endregion

#pragma region "strong udt's"
		using strong_x = strong<X>;
		// non initialized not allowed
		strong_x sx(X::ref);
		const char * DBJ_MAYBE( name ) = (X)sx;
#pragma endregion 
#pragma region native arrays
		using three_ints = ref_to_arr<int, 3>;
		int iarr[]{ 1,2,3 };
		three_ints iarr_ref = std::cref(iarr);

		using strong_ref_to_arr = strong< three_ints >;
		strong_ref_to_arr sra(std::cref(iarr));
		auto wot = (strong_ref_to_arr::value_type)sra ;

		const three_ints::type & DBJ_MAYBE(ii3) = wot.get();

#pragma endregion
	}
}

namespace dbj::samples {

	namespace any {

		using namespace std;
		template <typename T> class any_wrapper;

		template <typename T>
		class any_wrapper final
		{
			static_assert(!std::is_reference_v< std::remove_cv_t<T> >,
				"[dbj::any_wrapper] Can not use a reference type");

			static_assert(!std::is_array_v<std::remove_cv_t<T> >,
				"[dbj::any_wrapper] Can not use an array type");

			std::any any_;

		public:
			// types
			typedef any_wrapper type;
			typedef std::remove_cv_t<T> data_type;

			// we take references, pointers and a such ;)
			explicit any_wrapper(const data_type & ref) noexcept : any_(ref) {}
			// we take no r-references or values
			// any_wrapper(data_type & ref) noexcept = delete; // : any_(move(ref)) {	}

			any_wrapper() noexcept {	}
#if 0
			// copy
			any_wrapper(const any_wrapper& rhs) noexcept : any_(rhs.any_) { }
			any_wrapper& operator=(const any_wrapper& x) noexcept {
				this->any_ = x.any_;
				return *this;
			}
			// move
			any_wrapper(any_wrapper && rhs) noexcept
			{
				std::swap(this->any_, rhs.any_);
			}

			any_wrapper& operator=(any_wrapper&& rhs) noexcept {
				std::swap(this->any_, rhs.any_);
				return *this;
		}
			// destruct
			~any_wrapper() { this->any_.reset(); }
#endif
			// only if function is stored
			template<
				typename = std::enable_if_t< std::is_function_v< data_type >>,
				class... ArgTypes
			>
				invoke_result_t< data_type &, ArgTypes...>
				operator() (ArgTypes&&... args) const {
				if (!empty()) {
					return invoke(get(), forward<ArgTypes>(args)...);
				}
				else {
					throw dbj::Exception(" can not call on empty data wrapped ");
				}
			}

			constexpr data_type get() const {
				try {
					return any_cast<data_type>(this->any_);
				}
				catch (std::bad_any_cast & x) {
					throw dbj::Exception(x.what());
				}
			}

			constexpr bool empty() const noexcept {
				return !(this->any_).has_value();
			}

	};

		// factory methods ----------------------------------------

		template <
			typename T,
			typename ANYW = typename any::any_wrapper< T >
		>
			inline auto make(T val_)
			-> ANYW
		{
			// can make only from values
			//  see the ctor deleted
			return any::any_wrapper< T >(val_);
		};

		// template < typename T >	inline auto make(T && val_) = delete ;

		// returns std::array of any_wrapper's
		// of the same type T
		template <
			typename T,
			std::size_t N,
			typename ANYW = any::any_wrapper< T >,
			typename RETT = std::array< ANYW, N >,
			typename std_arr_t = RETT
		>
			inline auto range(const T(&arrf)[N])
			-> RETT
		{

			RETT rezult;
			std::size_t j = 0;

			for (auto && element : arrf) {
				// warning C28020: The expression '0<=_Param_(1)&&_Param_(1)<=1-1' is not true at this call.
				// why ?
				// calling the 'operator =' on dbj any_wrapper, so ... ?
				rezult[j] = any::make(element);
				j += 1;
			}
			return rezult;
		};

		// r-references to constants/literlas <-- no can do
		template <
			typename T,
			std::size_t N,
			typename ANYW = any::any_wrapper< T >,
			typename RETT = std::array< ANYW, N >,
			typename std_arr_t = RETT
		>
			inline auto range(const T(&&arrf)[N]) = delete;

		using dbj::console::out;
		template<typename T> inline void out(const any_wrapper<T> & anyt_)
		{
			out(anyt_.get());
		}
} // any

} // dbj::samples

DBJ_TEST_SPACE_OPEN(dbj_any_wrapper_testing)

DBJ_TEST_UNIT(dbj_any_wrapper)
{
	using namespace ::dbj::samples::any;
	try {
		const int int_arr[]{ 1,2,3 };
		// array must be const
		auto any_0 = DBJ_TEST_ATOM(range(int_arr));

		// only values --> auto any_2 = any::make("NO CAN DO!");

		// no temporaries --> auto any_2 = any::make(std::string{"YES CAN DO"});

		std::string not_a_temporary{ "YES CAN DO" };
		auto any_2 = DBJ_TEST_ATOM(make(not_a_temporary));

		auto DBJ_MAYBE(any_3) = DBJ_TEST_ATOM(any_2); // copy wrapper to wrapper
	}
	catch (...) {
		dbj::console::print(dbj::Exception(
			__FUNCSIG__ "  Unknown exception caught! "
		));
	}
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

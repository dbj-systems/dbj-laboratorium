#pragma once
#include "pch.h"
#include <optional>

#pragma region OPTIONAL WRAPPER
namespace dbj::optnl {

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
		using value_type = remove_cv_t<T>;
		// copied here from optional 
		// so that VS IDE can warn us *much* earlier
		// we are tryin to handle illegal types
		// with std::optional
		static_assert(!is_reference_v<value_type>,
			"value_type in strong<T> cannot be a reference type");
		static_assert(!is_same_v<value_type, nullopt_t>,
			"value_type in strong<T> cannot be nullopt_t");
		static_assert(!is_same_v<value_type, in_place_t>,
			"value_type in strong<T> cannot be in_place_t");
		static_assert( is_object_v<value_type>,
			"value_type in strong<T> must be an object type");
		static_assert( is_destructible_v<value_type> && !is_array_v<value_type>,
			"value_type in strong<T> must satisfy the requirements of Destructible");

	// must have some *non default*
	// initial value
	// MSVC says (2018 Oct) :
	// error C4700: uninitialized local variable used
	// using dbj strong<> that can not happen
	// no default ctor stops creation of T 
	// with default intrinsic values
		strong() = delete;

		strong(value_type arg) {
			value_.emplace(arg);
		}

		operator value_type & () noexcept {
			// _ASSERTE( value_ );	return *value_;
			// but, I prefer an exception when value_ is not initialized:
			return value_.value();
		}
	private:
		std::optional<value_type> value_;
	};

} // dbj::optnl
#pragma endregion
#pragma region ANY WRAPPER
namespace dbj {

	namespace any {

		using namespace std;
		template <typename T> class wrapper;

		template <typename T>
		class wrapper final
		{
			static_assert(!std::is_reference_v< std::remove_cv_t<T> >,
				"[dbj::wrapper] Can not hold a reference type");

			static_assert(!std::is_array_v<std::remove_cv_t<T> >,
				"[dbj::wrapper] Can not hold an array type");

			std::any any_;

		public:
			// types
			typedef wrapper any_wrapper_type;
			typedef std::remove_cv_t<T> value_type;

			// we take references, pointers and a such ;)
			explicit wrapper(const value_type & ref) noexcept : any_(ref) {}
			// we take no r-references or values
			// wrapper(value_type & ref) noexcept = delete; // : any_(move(ref)) {	}

			wrapper() noexcept {	}
#if 0
			// copy
			wrapper(const wrapper& rhs) noexcept : any_(rhs.any_) { }
			wrapper& operator=(const wrapper& x) noexcept {
				this->any_ = x.any_;
				return *this;
			}
			// move
			wrapper(wrapper && rhs) noexcept
			{
				std::swap(this->any_, rhs.any_);
			}

			wrapper& operator=(wrapper&& rhs) noexcept {
				std::swap(this->any_, rhs.any_);
				return *this;
		}
			// destruct
			~wrapper() { this->any_.reset(); }
#endif
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
		static std::array< ANYW, N > range(const T(&arrf)[N])
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


		// good example how to overload dbj::console::out for UDT
		using dbj::console::out;
		template<typename T> inline void out(const wrapper<T> & anyt_)
		{
			out(anyt_.get());
		}
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
#pragma region strong intrinsic
		{
			// usual "strong types" mantra:
			// "not just" int, float, size_t etc ...
			// but with some behaviour added
			// dbj micro strong types are this:
			using int_ = dbj::optnl::strong<int>;
			using float_ = dbj::optnl::strong<float>;
			using size_ = dbj::optnl::strong<unsigned int>;
			// interesting
			using void_ = dbj::optnl::strong<void>;
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

#pragma region strong udt-s
		{
			using strong_x = dbj::optnl::strong<X>;
			// non initialized not allowed
			strong_x sx(X::ref);
			const char * DBJ_MAYBE(name) = (X)sx;
		}
#pragma endregion 
	}

		DBJ_TEST_UNIT(dbj_any_wrapper)
	{
		using namespace ::dbj::any;
		try {
			int int_arr[]{ 1,2,3 };
			// makes std array of any wrappers from native array
			auto any_0 = DBJ_TEST_ATOM(range(int_arr));
			// only values --> auto any_2 = any::make("NO CAN DO!");
			// no temporaries --> auto any_2 = any::make(std::string{"YES CAN DO"});
			std::string not_a_temporary{ "YES CAN DO" };
			auto any_2 = DBJ_TEST_ATOM(make(not_a_temporary));
			auto DBJ_MAYBE(any_3) = DBJ_TEST_ATOM(any_2); // copy wrapper to wrapper
			// can we call a call operator?
			// no we can't --> auto rez_3 = any_3( 43 );
			// that fails to compile
			// but this won't
			auto any_4 = make([&](auto x) { return typeid(x).name();  });
			auto DBJ_MAYBE(rez_4) = any_4(true);
		}
		catch (...) {
			dbj::console::print(dbj::exception(
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

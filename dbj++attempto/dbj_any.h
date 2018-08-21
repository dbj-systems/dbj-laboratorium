#pragma once
#include "pch.h"


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

		std::any any_{};

	public:
		// types
		typedef any_wrapper type;
		typedef std::remove_cv_t<T> data_type;

		any_wrapper() noexcept {	}

		// we take references, pointers and a such ;)
		any_wrapper(const data_type & ref) noexcept : any_(ref) {}
		// we take no r-references or values
		any_wrapper(data_type && ref) noexcept = delete; // : any_(move(ref)) {	}

		// copy
		any_wrapper(const any_wrapper& rhs) noexcept : any_ ( rhs.any_ ) { }
		any_wrapper& operator=(const any_wrapper& x) noexcept {
				this->any_ = x.any_; 
			return *this;
		}
		// move
		any_wrapper(any_wrapper && rhs) noexcept 
		{  
			std::swap( this->any_ , rhs.any_) ;
		}

		any_wrapper& operator=(any_wrapper&& rhs) noexcept {
			std::swap(this->any_, rhs.any_);
			return *this ;
		}
		// destruct
		~any_wrapper() { this->any_.reset(); }

		// only if function is stored
		template< 
			typename = std::enable_if_t< std::is_function_v< data_type >> ,
			class... ArgTypes 
		>
		invoke_result_t< data_type &, ArgTypes...>
			operator() (ArgTypes&&... args) const {
			if (!empty()) {
				return invoke(get(), forward<ArgTypes>(args)...);
			} else {
				throw dbj::Exception(" can not call on empty data wrapped ");
			}
		}

		constexpr data_type get () const {
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
			typename ANYW = typename any::any_wrapper< std::remove_cv_t<T> >
		>
			inline auto make( T val_)
			-> ANYW
		{
			// can make only from values
			//  see the ctor deleted
			return ANYW{ val_ };
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
			-> std_arr_t
		{
			
			std_arr_t rezult{};
			std::size_t j{ 0 };

			for (auto && element : arrf) {
// warning C28020: The expression '0<=_Param_(1)&&_Param_(1)<=1-1' is not true at this call.
// why ?
// calling the 'operator =' on dbj any_wrapper, so ... ?
				rezult[j] = any::make(element);
				j += 1 ;
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

DBJ_TEST_SPACE_OPEN( dbj_any_wrapper_testing )

	DBJ_TEST_UNIT( dbj_any_wrapper) 
    {
		using namespace dbj::samples;
		try {
			const int int_arr[]{1,2,3};
			// array must be const
			auto any_0 = DBJ_TEST_ATOM( any::range(int_arr) );

			// only values --> auto any_2 = any::make("NO CAN DO!");

			// no temporaries --> auto any_2 = any::make(std::string{"YES CAN DO"});

			std::string not_a_temporary { "YES CAN DO" };
			auto any_2 = DBJ_TEST_ATOM( any::make("YES CAN DO"));

			auto  v1 = DBJ_TEST_ATOM( any_2 ); // copy wrapper to wrapper
			auto  v2 = DBJ_TEST_ATOM( v1.get() ); // wrapper to value and so on
		}	catch (...) {
			dbj::console::print( dbj::Exception(
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

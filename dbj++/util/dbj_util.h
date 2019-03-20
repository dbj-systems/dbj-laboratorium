#pragma once
// dbj++ 2018
// license at eof
/*
#include <type_traits>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <ratio>
*/
#include "../core/dbj_traits.h"
#include <set>
#include <unordered_set>

// basically forget about init lists passing
// https://stackoverflow.com/questions/20059061/having-trouble-passing-multiple-initializer-lists-to-variadic-function-template/47159747#47159747
// yes I know macro + variadic = horific
// but in here it seems as a sort of a solution
#define DBJ_IL(T,...) (std::initializer_list<T>{__VA_ARGS__})

namespace dbj {

	namespace util {
		// itox = integer to type x
		namespace itox {

			constexpr inline char itoc(unsigned k) noexcept { return char(k); }
#pragma region integer to digit char
			using itod_policy = char(*)(unsigned);

			// fold back to 0..9 if argument is larger than 9
			inline itod_policy fold_back =
				 [](unsigned k) constexpr -> char {	return '0' + char(k % 10); };

			inline itod_policy brutal =
				[](unsigned k) constexpr -> char    { _ASSERTE(k < 10);  return '0' + char(k % 10); };

			/*
			integer to digit
			*/
			constexpr inline char itod(unsigned k, itod_policy policy = fold_back) noexcept {
				return policy(k);
			}
#pragma endregion

		} //itox

#if (__cplusplus <= 201703L)
		/*
		http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0357r2.html

		Usage example:

		template<typename T, typename U>
		auto my_tie(T& t, U & u)
		{
		// new predefined macro for C++20 reference_wrapper
		// that allows for incomplete types to be used
		#if __cpp_lib_reference_wrapper >= 20YYMM
		  return std::make_pair(std::ref(t), std::ref(u));
		#else
		  return std::pair<T&, U&>(t, u);
		#endif
		}

		*/

		template<typename T>
		class reference_wrapper
		{
			T* ptr;

		public:
			using type = T;

			reference_wrapper(T& val) noexcept
				: ptr(std::addressof(val))
			{}

			reference_wrapper(T&&) = delete;

			T& get() const noexcept { return *ptr; }
			operator T&() const noexcept { return *ptr; }

			template<typename... Args>
			auto operator()(Args&&... args) const
				-> std::invoke_result_t<T&(Args...)>
			{
				return std::invoke(*ptr, std::forward<Args>(args)...);
			}
		};

#endif // __cplusplus <= 201703L

		// std::equal has many overloads
		// it is sometimes less error prone to have it here
		// in a singular form, and use this one as we exactly need
		template<class InputIt1, class InputIt2>
		constexpr bool equal_(InputIt1 first1, InputIt1 last1, InputIt2 first2)
		{
			for (; first1 != last1; ++first1, ++first2) {
				if (!(*first1 == *first2)) {
					return false;
				}
			}
			return true;
		}

		//-----------------------------------------------------------------------------
		/*

		why? to have "void" as function return type
		why? for std::cout << T situations
		how to use:

		void f1() {}
		const char * f2() { return __func__ ; }

		std::cout << std::endl <<
		std::get<0>( possible_void_call(f1) )
		<< std::endl ;

		std::cout << std::endl <<
		std::get<0>( possible_void_call(f2) )
		<< std::endl ;
		*/
		template<typename F, typename  ...Args >
		inline auto
			possible_void_call(F fun, Args ... args)
		{
			if constexpr (std::is_void<
				std::invoke_result_t<F, Args...>
			>::value)
			{
				return std::make_tuple("void");
			}
			else {
				return std::make_tuple(fun(args...));
			}
		}

		//-----------------------------------------------------------------------------
		// I prefer it to std::array ;)
		template< typename T, std::size_t N	>
		struct nano_range final
		{
			constexpr static const auto MAX_CAPACITY = BUFSIZ * 2 * 64; // ~64K 
			static_assert(N > 0);
			static_assert(N < MAX_CAPACITY );
			using type = nano_range;
			using value_type = T;
			// using data_ref = std::reference_wrapper<T[N]>;
			using data_ref = T(&)[N];
			constexpr T * begin() const { return value; }
			constexpr T * end() const { return value + N; }
			constexpr size_t size() const { return N; }
			// note: returning ref to array, not a pointer
			constexpr  data_ref data() const { return data_ref{ value }; }
			// yes data is public
			// if you need a foot gun
			// help yourself
			mutable T value[N]{};
		};

		inline auto random = [](int max_val, int min_val = 1) -> int {
			static auto initor = []() {
				std::srand((unsigned)std::time(nullptr)); return 0;
			}();
			return min_val + std::rand() / ((RAND_MAX + 1u) / max_val);
		};

		/*
		does not compile *if* 'range' has no begind and no end
		*/
		inline auto dbj_range_count = [](auto && range) constexpr->size_t
		{
			static_assert(::dbj::is_range_v< decltype(range) >)
				return std::distance(std::begin(range), std::end(range));
		};

		/*
		this is good idea: call F(), N times
		to create a tuple of N elements
		*/
		template <typename F, size_t... Is>
		inline auto gen_tuple_impl(F func, std::index_sequence<Is...>) {
			return std::make_tuple(func(Is)...);
		}

		template <size_t N, typename F>
		inline auto gen_tuple(F func) {
			return gen_tuple_impl(func, std::make_index_sequence<N>{});
		}

		// dbj added -- for std sequences
		// sequence into the tuple
		// sequence is anything that has an indexing operator
		template<typename T, size_t N>
		inline auto seq_tup(const T & sequence) {
			// call lambda N times
			return gen_tuple<N>(
				[&](size_t idx) { return sequence[idx]; }
			);
		};

		// dbj added -- for native arrays
		// native array into the tuple
		template<typename T, size_t N>
		inline auto seq_tup(T(&arr)[N]) {
			// call lambda N times
			return gen_tuple<N>(
				[&](size_t idx) { return arr[idx]; }
			);
		}

#if templated_classical_applicator
		/* non recursive applicator */
		template<typename F, typename... Targs>
		inline
			void applicator(F callback, Targs... args)
		{
			// since initializer lists guarantee sequencing, this can be used to
			// call a function on each element of a pack, in order:
			char dummy[sizeof...(Targs)] = { (callback(args), 0)... };
		}
#else
		/*applicator generic lambda*/
		inline auto applicator = [](auto  callback, auto... args)
			-> decltype(callback(args...))
		{
			return callback(args...);
		};
#endif

		/*
		find an item in anything that has begin and end iterators
		*/
		inline auto find = [](auto sequence, auto item)
			constexpr -> bool
		{
			return std::find(std::begin(sequence), std::end(sequence), item) != std::end(sequence);
		};


		/*
		------------------------------------------------------------------------------------
		*/
		template <typename Type, bool sort = false >
		inline std::vector<Type> remove_duplicates(const std::vector<Type> & vec)
		{
			_ASSERT(vec.size() > 0 );
			if constexpr (sort) {
				/*
				return it sorted and with no duplicates
				this is apparently also faster for very large data sets
				*/
				const std::set<Type> s(vec.begin(), vec.end());
				return std::vector<Type>{ s.begin(), s.end() };
			}
			else {
				/* just remove the duplicates */
				const std::unordered_set<Type> s(vec.begin(), vec.end());
				return std::vector<Type>{ s.begin(), s.end() };
			}
		}

		// return an dbj::smart_pair with
		// unique elements from the given sequence
		// and optionaly sorted
		template <typename Type >
			inline dbj::smart_pair<Type>
				remove_duplicates
					(Type const * left_, Type const *right_, bool sort = false )
						noexcept
		{
			_ASSERT(left_ && right_ );
			_ASSERT(left_ != right_ );

			auto make_return_value = [](auto a_, auto b_, size_t size_) {
				auto retval = dbj::make_smart_pair<Type>(size_) ;
				std::copy(a_, b_, retval.second.get());
				return retval ;
			};

			if (sort) {
				/*
				return it sorted and with no duplicates
				this is apparently also faster for very large data sets
				*/
				const std::set<Type> set_(left_, right_);
				return make_return_value( set_.begin(), set_.end(), set_.size() );
			}
			else {
				/* just remove the duplicates */
				const std::unordered_set<Type> set_(right_, right_);
				return make_return_value(set_.begin(), set_.end(), set_.size());
			}
		}

		// The memset function call could be optimized 
		// away by the compiler 
		// if the array object used, is no futher accessed.
		// other than this bellow  
		// one can use the more complex
		// std way: http://en.cppreference.com/w/c/string/byte/memset
		extern "C"
			inline void
			secure_zero(void *s, size_t n)
		{
#ifdef _DBJ_MT_
			dbj::sync::lock_unlock __dbj_auto_lock__;
#endif // _DBJ_MT_
			volatile char *p = (char *)s;
			while (n--) *p++ = 0;
		}

	} // util
} // dbj


/* inclusion of this file defines the kind of a licence used */
#include "..\dbj_gpl_license.h"
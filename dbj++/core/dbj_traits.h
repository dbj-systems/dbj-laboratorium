#pragma once
// license is at eof
/*
LINUX type name demangling has to be done like this

#include <cxxabi.h>

template < typemame T> 
std::string demangle () {
int error{0};
std::unique_ptr<char> 
     name = 
	   std::make_unique( abi::__cxa_demangle(typeid(T).name(), 0, 0, &error) );

if (!error)        return { name.get() };
if (error == -1)   return { "memory allocation failed" };
if (error == -2)   return { "not a valid mangled name" };
     return { "bad argument" };
}

*/

#include "dbj_crt.h"

#include <type_traits>
#include <utility>
#include <typeinfo>
#include <array>
#include <vector>
#include <string>

/*
 typeid() returns the outcome of  
 highly specific implementation
 we can not do anything meaningfull with it
 but these two simple macros
*/
#ifndef DBJ_TYPENAME
#define DBJ_TYPENAME(dbj_some_type_t_) ( typeid(dbj_some_type_t_).name() ) 
#define DBJ_VALTYPENAME(dbj_some_value_) ( typeid(decltype(dbj_some_value_)).name() ) 
#else
#error  DBJ_TYPENAME already defined?
#endif // !DBJ_TYPENAME

namespace dbj {

	/*
	currently MSVC compiler 2019-07-02 wrongly allows c++
	type casting from "anything" to function pointer
	so the bellow will not work untill that is fixed
	
	using FP = void (*)(int) ;
		template <FP fun> call_fp ( int ) { } 
			// should not compile but it does
	call_fp< FP(42) >( 0 );
	
	*/
#ifdef DBJ_MSVC_FIXED_FP_CASTING_FROM_ANYTHING

	/*
	if FP is a function pointer, anything can be cast by it
	but only some of the results will be callable

	using FP = int (*)() ;
	auto x = FP(42) ; // X will be of type FP
	x() ; // crash

	this is the tool to test before calling function pointers
	of seemingle the right type but of "unknown origin"
	*/
	template<typename FP>
	struct function_pointer final
	{
		static_assert(std::is_invocable_v<FP()>, "\n\nFP found not to be invocable\n");

		using value_type = FP;
		using type = function_pointer;
		using empty_type = std::invoke_result_t<FP()>;

		/*
		can not rely on the apparent FP type
		so let's assume it is the same but let us test
		for that in here
		*/
		template<FP  fun_candidate_, typename ...A>
		static constexpr bool is_callable (A...args)	
		{
			// static_assert does not work here
			// example
			// if F is int, int() is c++type cast and is invocable
			static_assert(sizeof(fun_candidate_(args...)), "\n\nFP found not to be invocable\n");
			static_assert(std::is_object_v<FP>, "\n\nFP found not to be an object type\n");
			// but this works
			// this will not compile if actual_fun_ is not a function
			// with signature that FP type describes
			// auto fun = static_cast<FP>(actual_fun_);
			auto   actual_fun_ = static_cast<FP>(fun_candidate_);
			return (actual_fun_ != nullptr);
		}

		/*applicator generic */
		template<typename ...A>
		static auto applicator (FP  callback, A... args)
			-> decltype(callback(args...))
		{
			return callback(args...);
		};
	};
#endif // DBJ_MSVC_FIXED_FP_CASTING_FROM_ANYTHING

	template< class T >
	struct ok_to_be_smart : std::integral_constant<
		bool,
		! std::is_class_v<T>  && ! std::is_union_v<T>  &&
		std::is_trivial_v<T>  && std::is_move_constructible_v<T>
	> {};

	template< class T >
	constexpr bool ok_to_be_smart_v = ok_to_be_smart<T>::value;


	/***********************************************************************************
	smart pointers of arrays are very usefull
	note 1: always use make_unique
	note 2: std::move () is the only way out for copying unique_ptr, moving works
	note 3: consider std::reference_wrapper<> to make unique_ptr as argument possible
	*/
	template<typename T_ , std::enable_if_t< ok_to_be_smart_v<T_>, int> = 0	>
	using unique_arr = std::unique_ptr<T_[]>;

	template<typename T_> 
	inline unique_arr<T_> make_unique_arr(size_t N_) {
		return std::make_unique<T_[]>(N_);
	}
	/*
	smart_pair<T> makes smart pointers of arrays even more usefull
	it keeps the count of array
	*/
	template<typename T_>
	using smart_pair =	std::pair < size_t, ::dbj::unique_arr<T_> >;

	template<typename T_>
	inline smart_pair<T_> make_smart_pair(size_t N_) {
		return std::make_pair( N_, std::make_unique<T_[]>(N_)) ;
	}

	/************************************************************************************/
	namespace tt {

  // usefull and important aliases
  // these make type traits more palatable

    /************************************************************************************/
	/* constraint: both types must be fundamental */
	template<typename T1, typename T2>
	struct is_fundamental_pair {
		
		using type_1 = std::remove_cv_t <T1>;
		using type_2 = std::remove_cv_t <T2>;

		constexpr static const bool value{
			std::is_fundamental_v<type_1> == std::is_fundamental_v<type_2>
		};
	};

	template<typename T1, typename T2>
	constexpr inline bool is_fundamental_pair_v = is_fundamental_pair<T1, T2>::value;

	template< class T1, class T2 >
	struct fundamental_twins final
		: std::bool_constant< 
		is_fundamental_pair_v<T1, T2> && std::is_same_v<std::decay_t<T1>, std::decay_t<T2>>
		> 
	{	
		using decay_1 = std::decay_t<T1>;
		using decay_2 = std::decay_t<T2>;
	};

	template<typename T1, typename T2>
	inline constexpr bool
		same_fundamental_types(T1&&, T2&&)
	{
		return fundamental_twins<T1, T2>{}();
	}
	/************************************************************************************/
	
	template<typename T>
	struct is_pointer_pointer final {
		enum : bool {
			value =
			std::is_pointer_v<T> &&
			std::is_pointer_v< typename std::remove_pointer_t<T> >
		};
	};

	template<typename T>
	constexpr auto is_pointer_pointer_v = is_pointer_pointer<T>::value;

	// to check the value
	inline auto is_p2p = [](auto arg) constexpr -> bool {
		using arg_type = decltype(arg);
		return is_pointer_pointer_v< arg_type >;
	};

	/************************************************************************************/

	template <typename T> struct remove_all_ptr { typedef T type; };

		template <typename T> struct remove_all_ptr<T*> {
			using type = typename remove_all_ptr<std::remove_cv_t<T>>::type;
		};

		// reduce T***** to T, for any level of pointers to pointers
		template <typename T>
		using remove_all_ptr_t = typename remove_all_ptr<T>::type;

		// std::remove_cvref is to be in the C++20
		template< class T >
		struct remove_cvref {
			typedef std::remove_cv_t<std::remove_reference_t<T>> type;
		};

		template< class T >
		using remove_cvref_t = typename remove_cvref<T>::type;

		// reduce any compound T , to its base type
		template <class T>
		using to_base_t =
			remove_all_ptr_t< std::remove_all_extents_t< remove_cvref_t < T > > >;

/************************************************************************************/
/* this can not be made to act at the compile time */
	template <class T1, class T2>
	inline bool same_typeid = typeid(T1).hash_code() == typeid(T2).hash_code();
/************************************************************************************/
/*
are two type-id's equal, for the two values provided ?
*/

inline auto equal_types 
	= [](auto & a, auto & b) 
		constexpr -> bool
{
using namespace std;
return is_same_v< remove_cvref_t<decltype(a)>, remove_cvref_t<decltype(b)> >;
};
/************************************************************************************/
// remove const-ness and/or volatility before comparing
// do not remove anything else
template<typename T1, typename T2>
	constexpr inline bool remove_cv_compare_types 
		= std::is_same_v< std::remove_cv_t<T1>, std::remove_cv_t<T2>  >;
/************************************************************************************/
/* see dbj_util tests for usage example */

#pragma region is pointer
	template <typename T> inline constexpr const bool pointer(T const &) noexcept { return false; }
	template <typename T> inline constexpr const bool pointer(T const *)  noexcept { return true; }

	// #define CRAZY_ALGORITHMS
#ifdef CRAZY_ALGORITHMS
	template<typename T>
	inline const bool is_ptr_(const T &) {

		const char * name_ = dbj::tt::name_<T>();
		char * first = (char*)name_;
		while (0x0 != first++) {
			if (*first == '*') return true;
		}
		return false;
	}
#endif
#pragma endregion 

// vs std::is_array works on values not types
// proof: https://godbolt.org/g/8skXRF

/* is object an instance of native array */
template< typename T, size_t N>
inline constexpr bool is_array_(
	const T(&specimen[[maybe_unused]])[N]
)	{	return true;	}

/* is object an pointer to native array */
template< typename T, size_t N>	
inline constexpr bool is_array_(
	const T(*specimen[[maybe_unused]])[N]
)	{	return true; }

/************************************************************************************/

	template<typename T>
	struct actual_type final {
		using unqualified	= std::remove_cv_t< T >;
		using not_ptr		= std::remove_pointer_t< T > ;
		using decayed		= std::decay_t< T >;
		using base          = to_base_t<T>;
	};

/************************************************************************************/

		template< typename T>
		struct instrument final
		{
			constexpr inline static const char space[]{ " " };

			template< typename TT>
			struct descriptor final
			{
				using type = TT;
				constexpr   static inline const bool	is_pointer 
					= std::is_pointer_v<TT>;
				constexpr 	static inline const bool	is_array 
					= std::is_array_v<TT>;
				constexpr 	static inline const size_t  number_of_dimension 
					= std::rank_v<TT>;
				constexpr 	static inline const size_t  first_extent 
					= std::extent_v<TT>;

				const std::string to_string() const noexcept
				{
					return DBJ::printf_to_buffer(
						"\n%-20s"
						"\n%-20s : %s / %s"
						"\n%-20s : %s -- %zu"
						"\n%-20s : %s -- %zu",
						DBJ_TYPENAME(typename descriptor::type),
						space, (is_pointer ? "Pointer" : "NOT Pointer"), (is_array ? "Array" : "NOT Array"),
						space, "dimensions, if array", number_of_dimension,
						space, "dimension[0] size, if array", first_extent
					);
				}
			};

			using def_type 
				= descriptor<T>;
			using actual_type 
				= descriptor<std::remove_cv_t< std::remove_pointer_t<T> >>;
			using under_type 
				= descriptor<typename std::remove_all_extents<T>::type>;

			const std::string to_string(void) noexcept
			{
				return
					std::string{ "\ndefault type" } + def_type{}.to_string() +
					"\nactual type"		+ actual_type{}.to_string() +
					"\nunderlying type" + under_type{}.to_string() ;
			}
		};
/************************************************************************************/
#pragma region TUPLE MANIPULATORS
		template<typename T1, typename T2>
		struct tuple_concat;

		template<typename... T1, typename... T2>
		struct tuple_concat<std::tuple<T1...>, std::tuple<T2...>>
		{
			using type = std::tuple<T1..., T2...>;
		};

		template<typename T, size_t n>
		struct tuple_n_concat;

		template<typename T>
		struct tuple_n_concat<T, 0>
		{
			using type = std::tuple<>;
		};

		template<typename T, size_t n>
		struct tuple_n_concat
		{
			using type = typename tuple_concat<
				typename tuple_n_concat<T, n - 1>::type,
				std::tuple<T>
			>::type;
		};

		template<typename T, size_t N>
		using array_as_tuple = tuple_n_concat<T, N>;

		template<typename T, size_t N>
		using array_as_tuple_t = typename tuple_n_concat<T, N>::type;

		/*
		   to map array to tuple type, somehow, one has to do this

		   int int_arr[3]
		   std::tuple<int,int,int>

		   for arbitrary arrays that tuple declaration might be very
		   difficult to write by hand. Thus the helper above was invented:

		   int int_arr[65535]{} ;
		   array_as_tuple_t<int,65535> very_large_tuple{} ;
		*/
#pragma endregion
	} // tt
} // dbj 
#pragma region char and string related traits
/************************************************************************************/
namespace dbj {

	using namespace std;

	// dbj.org 2018-07-03
	// NOTE: pointers are not char's
	// char *. wchar_t * .. are thus not chars	
	// take care of chars and their signed and unsigned forms
	// where 'char' means one of the four std char types

	template<class _Ty>	struct is_char : std::false_type {	};
	template<> struct is_char<char> : std::true_type {	};
	template<> struct is_char<signed char> : std::true_type {	};
	template<> struct is_char<unsigned char> : std::true_type {	};

	template<typename T> inline constexpr bool  is_char_v = is_char<T>::value;

	template<class _Ty>	struct is_wchar : std::false_type {	};
	template<> struct is_wchar<wchar_t> : std::true_type {	};

	template<typename T> inline constexpr bool  is_wchar_v = is_wchar<T>::value;

	template<class _Ty>	struct is_char16 : std::false_type {	};
	template<> struct is_char16<char16_t> : std::true_type {	};

	template<typename T> inline constexpr bool  is_char16_v = is_char16<T>::value;

	template<class _Ty>	struct is_char32 : std::false_type {	};
	template<> struct is_char32<char32_t> : std::true_type {	};

	template<typename T> inline constexpr bool  is_char32_v = is_char32<T>::value;
/************************************************************************************/
	template<typename T, typename base_t = dbj::tt::to_base_t<T> >
	struct is_wide_narrow_char :
		std::integral_constant
		<
		bool,
		is_char< base_t >::value || is_wchar<base_t>::value
		>
	{};

	template<typename T >
	inline constexpr bool  is_wide_narrow_char_v = is_wide_narrow_char<T>::value;

/************************************************************************************/
	// and one for all
	template<typename T, typename base_t = dbj::tt::to_base_t<T> >
	struct is_std_char :
		std::integral_constant
		<
		bool,
		is_char< base_t >::value || is_wchar<base_t>::value ||
		is_char16<base_t>::value || is_char32<base_t>::value
		>
	{};

	template<typename T >
	inline constexpr bool  is_std_char_v = is_std_char<T>::value;
/************************************************************************************/

	// is T, a standard string
	template< class T >
	struct is_std_string : integral_constant<bool,
		is_same<T, string    >::value ||
		is_same<T, wstring   >::value ||
		is_same<T, u16string >::value ||
		is_same<T, u32string >::value> {};

	template<typename T>
	inline constexpr bool  is_std_string_v = is_std_string<T>::value;

} // dbj
#pragma endregion

#pragma region enable_if helpers

	/*
	Templates are zealous eaters of types
	thus overloading and templates do not mix easily
	iso C++ comitee inveneted std::enable_if<>
	Which in turn makes you either "be somewhere else"
	or scramble for false cover of macros
	Here we do not use macros. we use modern C++ to deliver
	helpers when using templates woth or without overloading

	std::enable_if<> tamed with template aliases
	insipred with https://codereview.stackexchange.com/questions/71946/use-of-macros-to-aid-visual-parsing-of-sfinae-template-metaprogramming
	NOTE: C++17 onwards is required
	
	*/
namespace dbj {
		/*DT  stands for Decay Type
		we usualy decay the types here before using them
		see http://en.cppreference.com/w/cpp/types/decay
		*/
		template< typename T>
		using DT = std::decay_t<T>; // since C++14
									/*EIF stands for enable if*/
		template< bool pred >
		using EIF = typename std::enable_if_t< pred, int >;
	
	/* we define constexpr dbj::is_ function for every std::is_ */
	template< typename T>
	constexpr bool is_object() { return std::is_object_v< DT<T>>; }

	template< typename T>
	constexpr bool is_integral() { return std::is_integral_v< DT<T>>; }

	template< typename T>
	constexpr bool is_floating() { return std::is_floating_point< DT<T> >::value; }

	/* bellow are dbj::require_ templates we use through EIF<> */
	template< typename T>
	using require_integral = EIF< std::is_integral_v< DT<T> > >;

	template< typename T>
	using require_floating = EIF< std::is_floating_point< DT<T> >::value >;

	template< typename T>
	using require_object = EIF< std::is_object_v< DT<T> > >;

}
#pragma endregion 
// enable_if helpers

#pragma region type traits + generic lambdas

namespace dbj 
{
	using char_star = std::remove_cv_t< decltype("X") >;
	using wchar_star = std::remove_cv_t< decltype(L"X") >;

	/// <summary>
	/// c++ 17 generic lambdas have issues
	/// with required types of auto arguments
	/// in c++20 this will be fixed with new
	/// lambda arguments template declaration syntax
	/// </summary>
	namespace required
	{
		template<typename T>
		using is_uint64 = std::is_same< std::uint64_t, dbj::tt::to_base_t<T>>;

	} // required 


} // dbj
#pragma endregion 

#pragma region for containers
  /*
  dbj vector to touple optimization of  https://stackoverflow.com/posts/28411055/
  */
namespace dbj {

	//NOT here --> using namespace std;

	namespace inner {

		template <typename T, typename = void>
		struct is_range final : std::false_type {};

		template <typename T>
		struct is_range<T
			, std::void_t
			  <
			    decltype(std::declval<T>().begin()),
			    decltype(std::declval<T>().end())
			  >
		> final : std::true_type {};

		/*-----------------------------------------------------------*/
		/*
		full is_stl_container solution here: https://goo.gl/8ZQ5Xj
		*/
		template<typename T > struct is_std_array : public false_type {};

		/* this is not catching anything but a type array<t,n> */
		template<typename T, size_t N>
		struct is_std_array< array<T, N> > : public true_type {};

		/*-----------------------------------------------------------*/

		template<typename T> struct is_std_vector : public false_type {};

		/* this is not catching anything but a type vector<t> */
		template<typename T, typename A>
		struct is_std_vector< vector<T, A> > : public true_type {};
	}

	// type aliases and value aliases
	template<typename T>
	constexpr inline bool is_range_v = ::dbj::inner::is_range<T>::value;

	// back to normal
	// std::decay<T> does not remove constness
	template <typename T>
	using no_const_decayed = std::remove_const_t < std::decay_t<T> >;

	template<typename T>
	using is_range_t = typename inner::is_range<T>::type;

	template< typename T >
	using is_std_array_t = typename inner::is_std_array< no_const_decayed<T> >::type;

	template< typename T >
	constexpr inline bool is_std_array_v = inner::is_std_array< no_const_decayed<T> >::value;

	template< typename T>
	using is_std_vector_t = typename inner::is_std_vector< no_const_decayed<T> >::type;

	template< typename T>
	constexpr inline bool is_std_vector_v = inner::is_std_vector< no_const_decayed<T> >::value;

	//template <typename T>
	//using is_bool_t = std::is_same<no_const_decayed<T>, bool>::type ;

	template <typename T>
	constexpr inline bool is_bool_v 
		= std::is_same_v< no_const_decayed<T> >, bool>;

} // eof dbj
#pragma endregion

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
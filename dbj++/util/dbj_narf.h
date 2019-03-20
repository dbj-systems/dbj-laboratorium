#pragma once
/*
used through dbj++.h
thus dbj_crt.h is included before this
*/

#include <array>

/// <summary>
/// NARF == Native ARray reFerence
///
/// 2019 01 31 -- dbj --  all of this kind of a works but in a fragile manner
///                       this it is complicated to use and has to be refactored
///                       todo: make it into struct intead of namespace
///                       use std::unique_ptr<T[N]> wherever necessary
///                       same is for std::reference_wrapper<std::unique_ptr<T[N]>>
/// </summary>
namespace dbj::narf {

	//---------------------------------------------------------------
	// NARF core
	//
	// the key abstraction
	// ARF == Array ReFerrence
	// ARF contains instance of native array T[N]
	// ARF can be safely copied/moved arround
	// in essence one uses ARF to carry arround native arrays
	// unlike std::array
	// ARF easily delivers reference to native array T[N]&
	// example: for() can be easily applied to T[N]&
	//  for (auto & e : arf.get() ) {}
	//
	// std::reference_wrapper<T>
	// -- can not be moved, just copied
	// -- contains T *
	//    which thus might easily be a pointer to stack space
	//---------------------------------------------------------------
	template <typename T, std::size_t N>
	using wrapper = std::reference_wrapper<T[N]>;

	template<
		typename T,
		std::size_t N,
		typename ARR = std::array<T,N>
	>
		ARR to_std_arr
		(
			const wrapper<T, N> & wrap
		)
    {
		ARR rezult{};
		std::size_t  j{0};
		for_each(wrap, [&]( auto element ){  rezult[j++] = element; });
		return rezult;
	}

	template< typename T,	std::size_t N, typename ARR = std::array<T, N>	>
		ARR to_std_arr	(	wrapper<T, N> && wrap	) = delete;

	template<typename T, size_t N>
	constexpr auto begin(const wrapper<T, N> & wrp_) {
		return std::begin(wrp_.get());
	}
	
	template<typename T, size_t N> constexpr auto begin(wrapper<T, N> && wrp_) = delete;

	template<typename T, size_t N>
	constexpr auto end(const wrapper<T, N> & wrp_) {
		return std::end(wrp_.get());
	}

	template<typename T, size_t N> constexpr auto end( wrapper<T, N> && wrp_) = delete;


	/// <summary>
	/// returns std::reference_wrapper copy
	/// that contains reference to native array 
	/// this can be also used with init list
	/// <code>
	///  auto ari = 
	///  native_arr_ref({ 0,1,2,3,4,5,6,7,8,9 });
	///  auto arc = 
	///  native_arr_ref({ "Char array" });
	/// </code>
	/// </summary>
	template <
		typename T, 
		std::size_t N, 
		typename wrap_type = wrapper<T,N> 
	>
	constexpr wrap_type
		make( const T ( & native_arr)[N])
		// -> wrapper<T, N>
	{
		// effectively remove the ref + constness
		using nativarref =  std::remove_cv_t<T>(&)[N];

		static T cludge[N]{};

		std::copy(native_arr, native_arr + N, cludge);
		
		return std::ref(
			// effectively remove the ref + constness
			cludge
		);
	}

	template <
		typename T,
		std::size_t N,
		typename wrap_type = wrapper<T, N>
	>
		constexpr wrap_type make( T(&& native_arr)[N]) = delete;
	/// <summary>
	/// returns std::reference_wrapper copy
	/// that contains reference to native array 
	/// in turn contained inside the std::array
	/// *value* argument
	/// </summary>
	template<typename T, std::size_t N >
	constexpr auto
		make(const std::array<T, N> & std_arr)
		-> wrapper<T, N>
	{
		// effectively remove the ref + constness
		using nativarref = std::decay_t<T>(&)[N];

		// a stunt?
		return dbj::narf::make(
			(nativarref)*(std_arr.data())
		);
	}

	template<typename T, std::size_t N >
	constexpr auto
		make( std::array<T, N> && std_arr) = delete;

	/// <summary>
	/// the ARF size is the
	/// size of n-array it holds
	/// </summary>
	template<typename T, std::size_t N >
	constexpr std::size_t size( const wrapper<T, N> & narw_) {
		return N; //  array_size(narw_.get());
	}

	template<typename T, std::size_t N >
	constexpr std::size_t size( wrapper<T, N> && narw_) = delete;

	/// <summary>
	/// the NARF data
	/// is the reference of n-array it holds
	/// please note the difference to 
	/// the std::data() version for
	/// native arrays, which returns
	/// a pointer to n-array
	/// <code>
	/// template <class T, std::size_t N>
	///   constexpr T* data(T(&array)[N]) noexcept
	///  {
	///	   return array;
	///  }
	/// </code>
	/// to use bellow one will tend to use auto, but
	/// one will get different types in relation to 
	/// the declaration of the rezult
	/// <code>
	/// auto narf = dbj::narf::make("narf to Array of chars");
	/// auto native_array_pointer = dbj::narf::data(narf);
	/// decltype(auto) not_elegant_ref = dbj::narf::data(narf);
	/// auto & the_arr_ref = dbj::narf::data(narf);
	/// </code>
	/// </summary>
	template<
		typename T, 
		std::size_t N, 
		typename ARF = T(&)[N] 
	>
	constexpr ARF data
	(
		const wrapper<T, N> & narw_
	) 
	{
#if _DEBUG
		auto p1 = narw_.get() ;
		_ASSERTE(p1);
		// p1 might 'accidentaly' be ok
		// ditto, lets try once more
		auto p2 = narw_.get();
		_ASSERTE(p2);
#endif
		// and finnaly
		return narw_.get(); ;

		// same as 
		// return (ARF)narw_;
	}

	template< typename T,	std::size_t N,	typename ARF = T(&)[N]	>
		constexpr ARF data	(	wrapper<T, N> && narw_	) = delete;

	// homage to the range concept
	// auto [B,E] = dbj::narf::range( <some narf> )
	//
	template<typename T, std::size_t N>
	constexpr auto range(const wrapper<T, N> & narw_) {
		/*
		return std::array<T*, 2>{
			dbj::narf::begin(narw_), dbj::narf::end(narw_)
		};
		*/
		return std::make_pair(dbj::narf::begin(narw_), dbj::narf::end(narw_));
	}

	template<typename T, std::size_t N>	constexpr auto range( wrapper<T, N> && narw_) = delete;

	/// <summary>
	/// default for_each function
	/// note: REF is about references
	/// thus the changes will stay in 
	/// the containing native array after
	/// we leave this function
	/// 
	/// please make sure to read
	/// https://en.cppreference.com/w/cpp/algorithm/for_each
	/// to understand the std::for_each 
	/// requirements and behaviour
	/// </summary>
	template< typename T, size_t N, typename FUN >
	constexpr auto for_each(const wrapper<T, N> & arf, const FUN & fun_) {
		//const auto & B = begin(arf);
		//const auto & E = end(arf);
		auto[B, E] = range(arf);
		return std::for_each(B, E, fun_);
	}

	template< typename T, size_t N, typename FUN >
	constexpr auto for_each( wrapper<T, N> && arf, FUN && fun_) = delete;

	/// <summary>
	/// generic apply function
	/// callback signature:
	/// <code>
	/// [] ( size_t idx_ , const auto & element ) {}
	/// </code>
	/// </summary>
	template< typename T, size_t N, typename CBK >
	constexpr auto apply(
		const wrapper<T, N> &  arf,
		CBK output_function
	) {
		size_t j = 0;
		auto outfun_wrap = [&](/*const*/ T & element_) {
			output_function(j++, element_);
		};

		return dbj::narf::for_each(arf, outfun_wrap);
	}

	template< typename T, size_t N, typename CBK >
	constexpr auto apply(
		wrapper<T, N> &&  arf,
		CBK output_function
	) = delete;

} // dbj::narf

#include "../dbj_gpl_license.h"
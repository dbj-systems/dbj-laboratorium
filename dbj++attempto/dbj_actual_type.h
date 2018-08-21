#pragma once
#include "pch.h"

namespace dbj_samples {

	template<typename T>
	struct is_pointer_pointer final {
		using arg_type = std::decay_t< T >;
		enum : bool {
			value =
			std::is_pointer_v<arg_type> &&
			std::is_pointer_v< typename std::remove_pointer_t<arg_type> >
		} ;
	};

	template<typename T>
	constexpr auto is_pointer_pointer_v = is_pointer_pointer<T>::value;

	auto is_p2p = [](auto arg) constexpr -> bool {
		using arg_type = decltype(arg) ;
#if 0
		return std::is_pointer_v<arg_type> &&
			std::is_pointer_v< typename std::remove_pointer_t<arg_type> >;
#endif
		return is_pointer_pointer_v< arg_type >;
	};

	/// <summary>
	/// reveal the actual type of T
	/// </summary>
	template< typename T> struct actually 
	{
		typedef typename std::decay_t< T > value_type;

		enum class traits {
			is_lv_ref		= std::is_lvalue_reference_v<T>,
			is_rv_ref		= std::is_rvalue_reference_v<T>,
			is_array        = std::is_array_v<T>,
			is_pointer_		= std::is_pointer_v<T>,
			double_pointer	= is_pointer_pointer_v<T> 
		};
	};
	
	// template alias helper for the above
	template<typename T>
	using actual_type_t = typename actually<T>::value_type;

	/// <summary>
	/// return the actual type description in json format
	/// </summary>
	template < typename T>
	inline auto show_actual_type = []() 
		-> std::string 
	{
		auto add = [] (string & host_, const string & prompt_ , const string & val_ = "" ) {
			static char empty_[]{ "" };
			host_.append( prompt_  )
				.append( val_ );
			return add;
		};

		using at = actually< T > ;
		using actual_value_type = typename at::value_type;

		std::string tempo_{ BUFSIZ || 1024 }; // optimize so that heap mem alloc does not happen

		tempo_ = typeid(at).name();
		add( tempo_ , "\n {\n\t'value_type' :\t " , typeid(actual_value_type).name());
		add(tempo_, ", \n\t'is_lv_ref':\t", at::is_lv_ref ? "true" : "false");
		add(tempo_, ", \n\t'is_rv_ref':\t", at::is_rv_ref ? "true" : "false");
		add(tempo_, ", \n\t'array':\t", at::is_array ? "true" : "false");
		add(tempo_, ", \n\t'pointer':\t", at::is_pointer_ ? "true" : "false");
		add(tempo_, ", \n\t'pointer_pointer':\t", at::double_pointer ? "true" : "false");
		add(tempo_, "\n }\n");

		return tempo_.data() ;
	};
} // dbj

#ifdef DBJ_TESTING_ONAIR
/*
 Reasoning, motivation and testing
*/
namespace {

	auto lambada = []( auto arg ) {
		using naive_type = decltype(arg);
		dbj::log::print("\nNaive\n", dbj::show_actual_type<naive_type>());
		using the_type = dbj::actual_type_t< naive_type >;
		dbj::log::print("\nActual\n", dbj::show_actual_type<the_type>());
	};

	auto lambada_complicada = []( const auto & arg) {
		using naive_type = decltype(arg);
		dbj::log::print("\nNaive\n", dbj::show_actual_type<naive_type>());
		using the_type = dbj::actual_type_t< naive_type >;
		dbj::log::print("\nActual\n", dbj::show_actual_type<the_type>());
	};


	DBJ_TEST_UNIT(" dbj actual type; motivation and testing ") {

		const char * argv[]{ "ONE", "TWO", "THREE" };

		dbj::log::print("\n\n---------------------------------\nNow Calling lambada\n");
		lambada(argv);
		dbj::log::print("\n");

		dbj::log::print("\n\n---------------------------------\nNow Calling lambada complicada\n");
		lambada_complicada(argv);
		dbj::log::print("\n");

	}
}

#endif
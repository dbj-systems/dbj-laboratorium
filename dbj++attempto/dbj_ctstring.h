#pragma once
#include "pch.h"

namespace dbj_samples::ct {

	namespace  variadic_toolbox
	{
		template<unsigned  count,
			template<unsigned...> class  meta_functor, unsigned...  indices>
		struct  apply_range
		{
			typedef  typename apply_range<count - 1, meta_functor, count - 1, indices...>::result  result;
		};

		template<template<unsigned...> class  meta_functor, unsigned...  indices>
		struct  apply_range<0, meta_functor, indices...>
		{
			typedef  typename meta_functor<indices...>::result  result;
		};
	}

	namespace  compile_time
	{
		template<char...  str>
		struct  string
		{
			static  constexpr  const char  chars[] = { str..., '\0' };
		};

		template<char...  str>
		constexpr  const char string<str...>::chars[];

		template<typename  lambda_str_type>
		struct  string_builder
		{
			template<unsigned... indices>
			struct  produce
			{
				typedef  string < lambda_str_type().chars[indices]... > result;
			};
		};

		template<char...  str0, char...  str1>
		inline
			constexpr
			string<str0..., str1...>
			operator+(string<str0...>, string<str1...>)
		{
			return {};
		}
	}

	/*
	Q:why the macro? A: we can not pass 'string_literal'
	as lambda arg and then use it in seting up the 'string_struct'
	*/
#define  DBJ_CTSTRING(string_literal) \
[] () constexpr { \
	struct string_struct { \
		const char * chars = string_literal; \
	}; \
  return  variadic_toolbox::apply_range< \
	        sizeof(string_literal)-1, \
	  compile_time::string_builder < string_struct > ::produce \
		    >::result{}; \
}() 

#define	 DBJ_CTS_IMP(V, SL, SS ) \
struct SS {	const char * chars = SL; }; \
	constexpr auto V { \
		variadic_toolbox::apply_range< \
			sizeof(SL) - 1, \
			compile_time::string_builder < SS > ::produce \
		>::result{} };

#define DBJ_CTS(V, SL ) DBJ_CTS_IMP( V, SL, DBJ_CONCAT( the_struct_, __COUNTER__) )

	using namespace std::string_view_literals;

	DBJ_TEST_UNIT(_compile_time_string_)
	{
		constexpr auto sview{ "String View"sv };

		DBJ_CTS(cts1, "Hello " );
		DBJ_CTS(cts2, " World!");

		DBJ_TEST_ATOM(typeid(decltype(cts1)).name());


		auto  str_hello = DBJ_CTSTRING("hello");
		auto  str_world = DBJ_CTSTRING(" world");

		DBJ_TEST_ATOM(typeid(decltype(str_hello)).name());
		DBJ_TEST_ATOM(str_hello.chars);
		DBJ_TEST_ATOM((str_hello + str_world).chars);
	}
#undef DBJ_CTSTRING
} // dbj::ct
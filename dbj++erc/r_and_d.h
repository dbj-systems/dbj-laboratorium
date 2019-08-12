#pragma once
#include "pch.h"

#ifndef DBJ_LINE_U
#ifdef _MSC_VER
#define DBJ_LINE_U (cu_long)( _DBJ_CONCATENATE( __LINE__, U ) ) 
#else
#define DBJ_LINE_U (cu_long)( __LINE__ ) 
#endif
#endif

extern "C" {

#define MYCONSTEXPR constexpr 
	// #define MYCONSTEXPR  

	typedef struct MYID {
		unsigned long v_;
		const char* s_1;
		const char* s_2;
		// having a pointer in a struct, provokes
		// warning C4190:  'new_id' has C-linkage specified, but returns UDT 'ID' which is incompatible with C
		// *only* when returning struct from a constexpr function
	} MYID;

	inline constexpr MYID new_id(
		unsigned long seed_,
		const char* s_arg_1,
		const char* s_arg_2
	) {
		const MYID id_{ 1000UL + seed_, 0, 0 };
		return id_;
	}
} // eof "C"

constexpr MYID my_id_1 = new_id(__COUNTER__, 0, 0);
constexpr MYID my_id_2 = new_id(__COUNTER__, 0, 0);
// c++17
// Microsoft (R) C/C++ Optimizing Compiler Version 19.22.27905 for x86
DBJ_TEST_UNIT(constexpr_from_c)
{
	// warning C4190:  'new_id' has C-linkage specified, but returns UDT 'ID' which is incompatible with C
	constexpr auto id = my_id_1 ;
	constexpr auto id_v = my_id_2.v_ ;
}

#undef MYCONSTEXPR

extern "C" {
	/*
	since make_error is constexpr to use it with
	constexpr result it is very difficult to
	pass anything but string litterals (as
	const char *)  at compile time
	*/

	typedef 
#ifndef _MSC_VER
		const 
#endif
		unsigned long cu_long;
	// C compliant error_struct
	// aka POD in C++
	typedef struct error_struct
	{
		cu_long				err_id;
		const char* err_msg;
		cu_long				file_line;
		const char* file_path;
	} error_struct;

	// better 0 than casting -1 "special value" to unsigned long
	// it is very small chance the file line 0 will ever be
	// the location of the error
	constexpr cu_long not_file_line = (cu_long)(0);



	/*
	to return a struct is a legal C, but
	as of 2019JUL31, MSVC throws a warning here
		warning C4190:
		'make_error' has C-linkage specified,
		but returns UDT 'error_struct_' which is incompatible with C
		message :  see declaration of 'error_struct_'

		as of 2019JUL23, MSVC allows 'constexpr' keyword as here
	*/
	inline constexpr error_struct make_error(
		cu_long id_arg,
		const char msg_arg_ptr[BUFSIZ],
		cu_long line_arg,
		const char file_arg_ptr[BUFSIZ]
	) {
		assert(msg_arg_ptr != 0);

#ifndef _MSC_VER 
		/*
		as of 2019JUL23
		MSVC here requires /std:c++latest
		*/		return {
			.err_id = 43,.err_msg = "Wrong meaning",
			.file_line = DBJ_LINE_U,.file_path = __FILE__
		};
#else
		/*		following is fine		*/
		return
		{ id_arg, msg_arg_ptr , line_arg , file_arg_ptr };
#endif
	}

} // C code

	// this is MSVC /std:c++17 code
	// as of 2019JUL23
	// when following through debugger in VS2019 
	// make_error() return value appear to be
	// error_struct &
constexpr error_struct err1 =  /* with no location */
make_error(43, "Wrong meaning", not_file_line, 0);

constexpr error_struct err2 = /* with location included*/
make_error(42, "Ambiguous meaning", DBJ_LINE_U, __FILE__);

DBJ_TEST_UNIT(c_error_struct)
{
	auto see_it = [](auto err_)
	{
		auto id = err_.err_id;
		auto msg = err_.err_msg;
		auto line = err_.file_line;
		auto path = err_.file_path;
	};

	see_it(err1);
	see_it(err2);

	// try cheating and provoking a memory leak
	// can do
	// auto leaky = make_error(43, (new char[BUFSIZ]), not_file_line, 0);
	// no can do
	// constexpr auto leaky = make_error(43, (new char[BUFSIZ]), not_file_line, 0);
}

namespace experimental {

	//constexpr size_t string_literal_length(const char str[1])
	//{
	//	return *str ? 1 + string_literal_length(str + 1) : 0;
	//}
	// and the test
	constexpr std::array<char, dbj::str::string_literal_length("Hola Lola Loyola!") >
		my_simple_buffer{ { 0 } };

	//https://en.cppreference.com/w/cpp/experimental/to_array
	namespace detail {
		template <class T, std::size_t N, std::size_t... I>
		constexpr std::array<std::remove_cv_t<T>, N>
			to_array_impl(T(&a)[N], std::index_sequence<I...>)
		{
			return { {a[I]...} };
		}
	}

	template <class T, std::size_t N>
	constexpr std::array<std::remove_cv_t<T>, N> to_array(T(&a)[N])
	{
		return detail::to_array_impl(a, std::make_index_sequence<N>{});
	}
} // experimental

namespace array_capsule
{
	template <typename>	struct CString;

	template <unsigned N>
	struct CString  <std::array<char, N> >
	{
		std::array<char, N> const data;

		CString(std::array<char, N> const& arg_arr_) : data(arg_arr_) {}

		template<size_t N>
		CString(const char(&arg_arr_)[N]) : data(experimental::to_array(arg_arr_)) {}
	};

	// deduction guide 1
	template <std::size_t N>
	CString(std::array<char, N> const& pInput)->CString<std::array<char, N>>;

	// deduction guide 2
	template <std::size_t N>
	CString(const char(&literal_)[N])->CString<std::array<char, N>>;


	DBJ_TEST_UNIT(use_deduction_guides)
	{
		auto Meme = CString(experimental::to_array("Hola Lola!"));
		auto wot = CString("Hola Lola!");

		// std::cout << typeid(decltype(Meme.Label)).name() << "\n";
	}


#undef  ENCAPSULATE
}

namespace strong {
#define STRONG(N,T) struct N final { T v; }

	STRONG(Id, long);
	STRONG(Msg, std::string_view);

	struct id_msg_type final {
		mutable Id id;
		mutable Msg data;
	};

	using errType = id_msg_type;
	using locType = id_msg_type;

	struct errorType {
		errType err;
		locType loc;
	};

	constexpr errorType e1{ 42, "Hola Lola!" };
	constexpr errorType e2{ 43, "Hola Loyola!" };
	constexpr errorType e3{ 44, "Hola Ignacio!" };


#undef STRONG
} // strong

namespace r_and_d {
	DBJ_TEST_UNIT(compile_time_sv_carier)
	{
		using dbj::fmt::print;
		using namespace std::literals;

#define sv_carrier(L_) [] () constexpr { constexpr auto sview = _DBJ_CONCATENATE(L_,sv); return sview; }

		// but this also works
		constexpr auto carrier = sv_carrier("Hola Loyola!");
		print("\n\n%s", carrier().data());

#undef sv_carrier
	}

	// Convert array into std::array
	template<size_t N, std::size_t... I>
	inline constexpr decltype(auto)
		charr_to_stdarr(const char(&charr)[N], std::index_sequence<I...>)
	{
		return std::array<char, N>{ { charr[I]...} };
	}

	/*
	from string literal make compile time char buffer inside std::array

	constexpr auto buffer_1 = inner::char_buff("Hola Lola!");
	constexpr std::array<char, buffer_1.size() > buffer_2 = buffer_1;
	*/
	template<size_t N, typename Indices = std::make_index_sequence<N> >
	constexpr decltype(auto)
		char_buff(const char(&sl_)[N])
	{
		//std::array<char, N + 1 > buffer_{ { 0 } };
		//size_t k = 0;
		//for (auto chr : sl_) { buffer_[k++] = chr; }
		//return buffer_;

		return charr_to_stdarr(sl_, Indices{});
	}

	DBJ_TEST_UNIT(constexpr_charr_array_carier)
	{
#define buf_carrier(L_) [] () constexpr { constexpr auto char_arr = char_buff(L_); return char_arr; }

		constexpr auto chr_arr_buf = char_buff("Hola Ignacio!");
		using dbj::fmt::print;
		print("\n\n%s", chr_arr_buf.data());

#undef buf_carrier
	}
} // ns

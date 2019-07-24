#pragma once
#include "pch.h"

extern "C" {

	typedef const unsigned long cu_long_type ;
	// C compliant error_struct
	typedef struct error_struct_ 
	{
		cu_long_type		err_id;
		const char *		err_msg;
		cu_long_type		file_line;
		const char *		file_path;
	} error_struct ;

	static cu_long_type not_file_line = (cu_long_type)(-1);

#define DBJ_LINE_U (cu_long_type)( _DBJ_CONCATENATE( __LINE__, U ) ) 

	/*
	to return a struct is a legal C, but
	as of 2019JUL23, MSVC throws a warning here
	    warning C4190:  
		'test_c_compliant_error_struct' has C-linkage specified, 
		but returns UDT 'error_struct_' which is incompatible with C
        message :  see declaration of 'error_struct_'

		as of 2019JUL23, MSVC allows 'constexpr' keyowrd as here
	*/
	constexpr error_struct test_c_compliant_error_struct(
		cu_long_type id_arg,
		const char* msg_arg_ptr,
		cu_long_type line_arg,
		const char* file_arg_ptr
	) {
		assert(msg_arg_ptr != 0);
		/*
		as of 2019JUL23 
		for MSVC this requires /std:c++latest

		error_struct err1 = { 
			.err_id = 43, .err_msg = "Wrong meaning", 
			.file_line = DBJ_LINE_U, .file_path = __FILE__
		};

		following is fine
		*/
		error_struct err1 = 
		{ id_arg, msg_arg_ptr , line_arg , file_arg_ptr };

		return err1;
	}

} // C code

DBJ_TEST_UNIT(c_error_struct)
{
	// this is MSVC /std:c++17 code
	// as of 2019JUL23
	// when following through debugger in VS2019 
	// test_c_compliant_error_struct() return value appear to be
	// error_struct &
	constexpr error_struct err1 =  /* with no location */
		test_c_compliant_error_struct
			(43, "Wrong meaning", not_file_line, 0);

	constexpr error_struct err2 = /* with location included*/
		test_c_compliant_error_struct
			(42, "Anbiguous meaning", DBJ_LINE_U, __FILE__);
}

namespace experimental {

	constexpr size_t length(const char* str)
	{
		return *str ? 1 + length(str + 1) : 0;
	}
	// and the test
	constexpr std::array<char, length("Hola Lola Loyola!") > my_simple_buffer{ { 0 } };

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

		CString(std::array<char, N> const & arg_arr_) : data(arg_arr_) {}

		template<size_t N>
		CString(const char (&arg_arr_)[N]) : data(experimental::to_array(arg_arr_)) {}
	};

	// deduction guide 1
	template <std::size_t N>
	CString(std::array<char, N> const& pInput)->CString<std::array<char, N>>;

	// deduction guide 2
	template <std::size_t N>
	CString(const char (&literal_)[N]) -> CString<std::array<char, N>>;


	DBJ_TEST_UNIT( use_deduction_guides )
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
	} ;

	using errType = id_msg_type;
	using locType = id_msg_type;

	struct errorType {
		errType err;
		locType loc;
	};

	constexpr errorType e1{ 42, "Hola Lola!"};
	constexpr errorType e2{ 43, "Hola Loyola!"};
	constexpr errorType e3{ 44, "Hola Ignacio!"};


#undef STRONG
} // strong

namespace r_and_d {
	DBJ_TEST_UNIT(compile_time_sv_carier)
	{
		using dbj::fmt::print;
		using namespace std::literals;

#define sv_carrier(L_) [] () constexpr { constexpr auto sview = DBJ_CONCAT(L_,sv); return sview; }

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

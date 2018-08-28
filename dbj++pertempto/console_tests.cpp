// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"

// **********************************************************************************************
DBJ_TEST_SPACE_OPEN(console_test)
// **********************************************************************************************


extern "C"  void console_painting()
{ 
	using namespace dbj;

	constexpr const dbj::c_line<80, '+'> L80{};

	console::print(
	    console::nl, console::painter_command::nop, "NOP", L80 , 
		console::nl, console::painter_command::text_color_reset, "RESET", L80 ,
		console::nl, console::painter_command::white, "WHITE", L80 ,
		console::nl, console::painter_command::red, "RED", L80 ,
		console::nl, console::painter_command::green, "GREEN", L80 ,
		console::nl, console::painter_command::blue, "BLUE", L80 ,
		console::nl, console::painter_command::bright_red, "BRIGHT_RED", L80 ,
		console::nl, console::painter_command::bright_blue, "BRIGHT_BLUE", L80 ,
		console::nl, console::painter_command::text_color_reset, "RESET"
		);
}

/// <summary>
/// https://en.cppreference.com/w/cpp/language/types
/// </summary>
extern "C"  void fundamental_types_to_console()
{
	using dbj::console::out;
	using dbj::console::PRN ;
	using namespace std;

	PRN.printf("\n");
	out((nullptr_t) nullptr);
	PRN.printf("\n");
	out(true);

	// std chars
	PRN.printf("\n");
	out((signed char) 'X');
	PRN.printf("\n");
	out((unsigned char) 'X');
	PRN.printf("\n");
	out((char) 'X');
	PRN.printf("\n");
	out((wchar_t) 'X');
	PRN.printf("\n");
	out((char16_t) 'X');
	PRN.printf("\n");
	out((char32_t) 'X');
	// integer types
	PRN.printf("\n");
	out((short    int) 'X');
	PRN.printf("\n");
	out((unsigned short int) 'X');
	PRN.printf("\n");
	out((int) 'X');
	PRN.printf("\n");
	out((unsigned       int) 'X');
	PRN.printf("\n");
	out((long    int) 'X');
	PRN.printf("\n");
	out((unsigned long  int) 'X');
	PRN.printf("\n");
	out((long     long  int) 'X');
	PRN.printf("\n");
	out((unsigned long  long  int) 'X');
	// float types
	PRN.printf("\n");
	out((float) 42.99);
	PRN.printf("\n");
	out((double) 42.99);
	PRN.printf("\n");
	out((long double) 42.99);
	// nan float's
	PRN.printf("\n");
	out((float)			nanf("1"));
	PRN.printf("\n");
	out((double)		nan ("1"));
	PRN.printf("\n");
	out((long double)	nanl ("1"));
}

template< typename T>
void string_tester ( T * init_str_literal)
{
	using namespace std;

	// DBJ_TYPE_REPORT_FUNCSIG;

	auto actual_tests = [&](auto && std_string) 
	{
		using dbj::console::out;
		using dbj::console::PRN;
		// using namespace dbj::tt;
		using namespace std;

		wstring wstr_ = dbj::range_to_wstring(std_string);
#if 0 // avoid the print()
		// print to the out() overloads
		PRN.wchar_to_console(L"\n");
		out(std_string);
		PRN.wchar_to_console(L"\n");
		out(std_string.data());
		// straight to the Printer
	
		PRN.wchar_to_console(L"\n");
		PRN.wchar_to_console(wstr_.data());
		// straight to the IConsole implementation
		PRN.wchar_to_console(L"\n");
		PRN.cons()->out(wstr_.data(), wstr_.data() + wstr_.size());
#else
		dbj::console::print( 
			dbj::console::nl, std_string, dbj::console::nl, std_string.data(), 
			dbj::console::nl, wstr_.data()
			);
#endif
	};

	using base_type = dbj::tt::to_base_t<T>;

	if constexpr (std::is_same_v <base_type, char>) {
		actual_tests(string{ init_str_literal });
		actual_tests(string_view{ init_str_literal });
	}
	else
		if constexpr (std::is_same_v<base_type, wchar_t>) {
			actual_tests(wstring{ init_str_literal });
			actual_tests(wstring_view{ init_str_literal });
		}
		else
			if constexpr (std::is_same_v<base_type, char16_t> ) {
				actual_tests(u16string{ init_str_literal });
				actual_tests(u16string_view{ init_str_literal });
			}
			else
				if constexpr (std::is_same_v<base_type, char32_t>) {
					actual_tests(u32string{ init_str_literal });
					actual_tests(u32string_view{ init_str_literal });
				}
				else {
					DBJ_UNHANDLED_("unknown argument type inside string_tester");
				}

};
/// <summary>
/// C++ has inhertied from C the special status of strings
/// which is unfortunate
/// </summary>
extern "C"  void strings_to_console()
{
	string_tester("narrow string");
	string_tester(L"wide string");
	string_tester(u"u16string string");
	string_tester(U"u32string string");

}

// **********************************************************************************************


template< typename ... Args >
void arh_test ( Args ... args)
{
	using dbj::console::out;
	using dbj::console::PRN;
	using namespace std;

	if constexpr( 1 > (sizeof... (args))  ) {
		return;
	}
	else 
	{
		auto	arg_list = { args ... };
		using	arg_list_type = decltype(arg_list);

		using arh_type = typename dbj::arr::ARH<arg_list_type::value_type, (sizeof... (args))>;

		static_assert(
			std::is_same_v< arh_type::value_type, arg_list_type::value_type >,
			"dbj::ARH::value_type must be the same to the type of each brace init list element"
			);

		typename arh_type::ARR std_arr{ { args ... } };
		dbj::console::print( 
			"\n", std_arr,
			"\n", arh_type::to_arp(std_arr),
			"\n", arh_type::to_arf(std_arr)
		);
	}

};

extern "C" void compound_types_to_console() 
{
	using namespace dbj::console;
	using namespace std;
	// array of fundamental types
	arh_test(1, 2, 3, 4, 5, 6, 7, 8, 9);
	// arrays of class types
	arh_test( string{ "ONE" }, string{ "TWO" }, string{ "THREE" });
	// pointers to fundamental types
	arh_test(L"ONE", L"TWO", L"THREE");
	// pointers out -- function pointer 
	print("\nstd::addressof( arh_test<int> )", std::addressof( arh_test<int> ));
	// init list
	auto init_list = {1,2,3,4,5,6,7,8,9,0};
	print("\ninit list:\t",init_list);
	//
	print(
	dbj::Exception	{ "\n\tdbj::Exception" },
	std::exception	{ "\n\tstd::exception" },
	std::runtime_error{ "\n\tstd::runtime_error" },
		"\nstd::vector<int>{1,2,3}:\t",  std::vector<int>{1,2,3} ,
		"\nstd::array<int, 3>{1,2,3}:\t", std::array<int, 3>{1,2,3},
		"\nstd::map<bool, int>{ {true,1}, {false,2}, {true,3}}:\t ", std::map<bool, int>{ {true,1}, {false,2}, {true,3}},
		"\nstd::variant<bool>{true}:\t", std::variant<bool>{true},
		"\nstd::make_tuple(1,true,42.56):\t", std::make_tuple(1,true,42.56),
		"\nstd::make_pair(1,2.9):\t", std::make_pair(1,2.9)
	);
}

DBJ_TEST_UNIT(dbj_console_testing)
{
	console_painting();
	strings_to_console();
	fundamental_types_to_console();
	compound_types_to_console();
}

DBJ_TEST_SPACE_CLOSE

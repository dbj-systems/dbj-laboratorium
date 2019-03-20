#pragma once

DBJ_TEST_SPACE_OPEN(_console_)

	DBJ_TEST_UNIT(_dbj_console_test_1 ) {

	using namespace  dbj::console;

		/*
		FONT is the key
		One has to use the font in the console which is
		capable of showing all the unicode as required
		*/

		constexpr wchar_t  doubles[]{ L"║═╚" };
		constexpr wchar_t  singles[]{ L"│─└" };
		constexpr wchar_t  squiglies[]{ L"αß©∂€αß©∂€αß©∂€αß©∂€αß©∂€" };

		auto run_them_through_colours = [&]( auto && text_ ) {
			/*
			here we use them commands
			*/
			dbj::console::print( "\n",
				"\n", painter_command::white, "White\t\t", text_,
				"\n", painter_command::red, "Red\t\t", text_,
				"\n", painter_command::green, "Green\t\t", text_,
				"\n", painter_command::blue, "Blue\t\t", text_,
				"\n", painter_command::bright_red, "Bright Red\t", text_,
				"\n", painter_command::text_color_reset, "Reset\t\t", text_
			);
		};
			run_them_through_colours(doubles);
			run_them_through_colours(singles);
			run_them_through_colours(squiglies);
	}
#if 0
	DBJ_TEST_UNIT(" : dbj wcout test TWO")
	{
/*
http://stackoverflow.com/questions/2492077/output-unicode-strings-in-windows-console-app
*/
		_setmode(_fileno(stdout), _O_U16TEXT);
		std::wcout << L"Testing unicode -- English -- Ελληνικά -- Español." << std::endl
			<< doubles << std::endl
			<< singles << std::endl;

		/*
		apparently must do this
		also make sure NOT to mix cout and wcout in the same executable
		*/
		std::wcout << std::flush;
		fflush(stdout);
		_setmode(_fileno(stdout), _O_TEXT);
	}
#endif

	extern "C"  
		inline void console_painting()
	{
		using namespace dbj;

		static const auto L80 = dbj::str::char_line('+');

		console::print(
			console::nl, console::painter_command::nop, "NOP", L80,
			console::nl, console::painter_command::text_color_reset, "RESET", L80,
			console::nl, console::painter_command::white, "WHITE", L80,
			console::nl, console::painter_command::red, "RED", L80,
			console::nl, console::painter_command::green, "GREEN", L80,
			console::nl, console::painter_command::blue, "BLUE", L80,
			console::nl, console::painter_command::bright_red, "BRIGHT_RED", L80,
			console::nl, console::painter_command::bright_blue, "BRIGHT_BLUE", L80,
			console::nl, console::painter_command::text_color_reset, "RESET"
		);
	}

	/// <summary>
	/// https://en.cppreference.com/w/cpp/language/types
	/// </summary>
	extern "C" 
		inline void fundamental_types_to_console()
	{
		using dbj::console::out;
		using dbj::console::PRN;
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
		out((float)nanf("1"));
		PRN.printf("\n");
		out((double)nan("1"));
		PRN.printf("\n");
		out((long double)nanl("1"));
	}

	template< typename T>
	inline void string_tester(T * init_str_literal)
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
				if constexpr (std::is_same_v<base_type, char16_t>) {
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
	extern "C" 
		inline void strings_to_console()
	{
		string_tester("narrow string");
		string_tester(L"wide string");
		string_tester(u"u16string string");
		string_tester(U"u32string string");

	}

	// **********************************************************************************************
	template< typename ... Args >
	inline void arh_test(Args ... args)
	{
		using dbj::console::out;
		using dbj::console::PRN;
		using namespace std;

		if constexpr (1 > (sizeof... (args))) {
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

	extern "C" inline void compound_types_to_console()
	{
		using namespace ::dbj::console;
		using namespace ::std;
		using namespace ::std::string_view_literals;
		// array of fundamental types
		arh_test(1, 2, 3, 4, 5, 6, 7, 8, 9);
		// arrays of class types
		arh_test(string{ "ONE" }, string{ "TWO" }, string{ "THREE" });
		// pointers to fundamental types
		arh_test(L"ONE", L"TWO", L"THREE");
		// pointers out -- function pointer 
		print("\nstd::addressof( arh_test<int> )", std::addressof(arh_test<int>));
		// init list
		auto init_list = { 1,2,3,4,5,6,7,8,9,0 };
		print("\ninit list:\t", init_list);
		//
		print(
			// ::dbj::exception( "\n\tdbj::Exception"sv ),
			std::exception{ "\n\tstd::exception" },
			std::runtime_error{ "\n\tstd::runtime_error" },
			"\nstd::vector<int>{1,2,3}:\t", std::vector<int>{1, 2, 3},
			"\nstd::array<int, 3>{1,2,3}:\t", std::array<int, 3>{1, 2, 3},
			"\nstd::map<bool, int>{ {true,1}, {false,2}, {true,3}}:\t ", std::map<bool, int>{ {true, 1}, { false,2 }, { true,3 }},
			"\nstd::variant<bool>{true}:\t", std::variant<bool>{true},
			"\nstd::make_tuple(1,true,42.56):\t", std::make_tuple(1, true, 42.56),
			"\nstd::make_pair(1,2.9):\t", std::make_pair(1, 2.9)
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

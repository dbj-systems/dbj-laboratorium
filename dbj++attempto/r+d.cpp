﻿// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// testing the various stuff in this project
#include "pch.h"
#include "dbj_lambda_lists.h"
#include "dbj_tokenizer.h"
#include "dbj_atoms.h"
#include "dbj_lambda_lists.h"
#include "dbj_any_node.h"
#include "dbj_tree_tests.h"

#include "string_literals_with_universal_character_names.h"
#include "no_copy_no_move.h"

#include <test/dbj_kalends_test.h>
#include <test/dbj_string_util_test.h>
#include <test/dbj_kv_store_test.h>

#include <string_view>

DBJ_TEST_SPACE_OPEN(local_tests)

/**************************************************************************************************/
using namespace std::literals;

struct STANDARD {
	constexpr static const auto compiletime_static_string_view_constant()
	{
		static auto make_once_and_only_if_called = "constexpr string view literal"sv;
		// on second and all the other calls 
		// just return
		return make_once_and_only_if_called;
	}
};

DBJ_TEST_UNIT(compiletime_static_string_constant)
{
	auto return_by_val = []() {
		auto return_by_val = []() {
			auto return_by_val = []() {
				auto return_by_val = []() {
					return STANDARD::compiletime_static_string_view_constant();
				};
				return return_by_val();
			};
			return return_by_val();
		};
		return return_by_val();
	};

	// std artefacts conformance
	auto the_constant = return_by_val();

	_ASSERTE(the_constant == "constexpr string view literal");

	static_assert(
		STANDARD::compiletime_static_string_view_constant()
		== "constexpr string view literal"
		);

	// make init list
	auto ref_w = { the_constant };
	// make vector
	const std::vector <char> vcarr{
		the_constant.data(), the_constant.data() + the_constant.size()
	};

	auto where = the_constant.find('e');
}
/**************************************************************************************************/

template< typename T>
void array_analyzer(const T & specimen) {

	static char const * name{ DBJ_TYPENAME(T) }; //safe?
	constexpr bool is_array = std::is_array_v<T>;
	if constexpr (is_array == true)
	{
		constexpr size_t number_of_dimension = std::rank_v<T>;
		constexpr size_t first_extent = std::extent_v<T>;
		std::wprintf(L"\n%S is %s", name, L"Array");
		std::wprintf(L"\n%-20S number of dimension is %zu", name, number_of_dimension);
		std::wprintf(L"\n%-20S size along the first dimension is %zu", name, first_extent);
	}
	else {
		std::wprintf(L"\n%S is %s", name, L"Not an Array");
	}
};

#define DBJ_IS_ARR(x) try_array( std::addressof(x) )

template<typename T>
constexpr auto try_array(T * specimen)  -> size_t {
	return  std::extent_v< T >;
}

template<typename T>
constexpr auto probe_array(T specimen)   -> size_t {
	return  std::extent_v< T >;
}

DBJ_TEST_UNIT(_array_stays_array)
{
	static int ia[]{ 1,2,3,4,5,6,7,8,9,0 };
	auto s0 = try_array(ia);
	auto s1 = DBJ_IS_ARR(ia);
	auto s2 = probe_array(ia);
	auto s3 = probe_array(std::addressof(ia));
}

typedef enum class CODE : UINT {
	page_1252 = 1252u,   // western european windows
	page_65001 = 65001u // utf8
	// page_1200 = 1200,  // utf16?
	// page_1201 = 1201   // utf16 big endian?
} CODE_PAGE;

DBJ_TEST_UNIT(_famous_dbj_console_ucrt_crash)
{
	// кошка 日本
	constexpr wchar_t specimen[] =
	{ L"\x043a\x043e\x0448\x043a\x0430 \x65e5\x672c\x56fd" };

	dbj::console::print("\n", specimen, "\n");

	// 1252u or 65001u
	if (::IsValidCodePage(65001u)) {
		auto scocp_rezult = ::SetConsoleOutputCP(65001u);
		_ASSERTE(scocp_rezult != 0);
	}
	/*
	<fcntl.h>
	_O_U16TEXT, _O_U8TEXT, or _O_WTEXT
	to enable Unicode mode
	_O_TEXT to "translated mode" aka ANSI
	_O_BINARY sets binary (untranslated) mode,
	*/
	int result = _setmode(_fileno(stdout), _O_U8TEXT);
	_ASSERTE(result != -1);

	// should display: кошка 日本
	// for any mode the second word is not displayed ?
	auto fwp_rezult = fwprintf(stdout, L"\nwfprintf() displays: %s\n", specimen);
	// for any mode the following crashes the UCRT (aka Universal CRT)
	// fprintf( stdout, "\nprintf() result: %S\n",specimen);
}


DBJ_TEST_UNIT(tokenizer_test)
{
	/* engine testing
	{
		dbj::samples::internal::tokenizer_eng	stok{ "abra ka dabra", " " };
		dbj::samples::internal::wtokenizer_eng	wtok{ L"abra ka dabra", L" " };

		auto s_ = stok.size();

		auto w0_ = stok.getWord(0);
		auto w1_ = stok.getWord(1);
		auto w2_ = stok.getWord(2);
	}
	*/

	auto test_tokenizer_moving_copying
		= [](auto src, auto token)
	{
		using base_type = dbj::tt::to_base_t< decltype(src) >;

		if constexpr (std::is_same_v< base_type, char>) {
			return dbj::pair_stokenizer{ src, token };
		}
		else
			if constexpr (std::is_same_v< base_type, wchar_t>) {
				return dbj::pair_wtokenizer{ src, token };
			}
			else {
				throw "char or wchar_t only please";
			}
	};

	auto test_tokenizer_usage = [](auto tizer)
	{
		for (auto && pos_pair : tizer) {
			auto w_ = DBJ_TEST_ATOM(tizer[pos_pair]);
		}
	};

	test_tokenizer_usage(
		test_tokenizer_moving_copying(R"(abra\nka\ndabra)", R"(\n)")
	);
	test_tokenizer_usage(
		test_tokenizer_moving_copying(LR"(abra\nka\ndabra)", LR"(\n)")
	);

	auto word_tokenizer_moving_copying
		= [](auto src, auto token)
	{
		using base_type = dbj::tt::to_base_t< decltype(src) >;

		if constexpr (std::is_same_v< base_type, char>) {
			return dbj::word_stokenizer{ src, token };
		}
		else
			if constexpr (std::is_same_v< base_type, wchar_t>) {
				return dbj::word_wtokenizer{ src, token };
			}
			else {
				throw "char or wchar_t only please";
			}

	};

	auto word_tokenizer_usage = [](auto tizer)
	{
		for (auto && word_ : tizer) {
			auto w_ = DBJ_TEST_ATOM(word_);
		}
	};

	word_tokenizer_usage(
		word_tokenizer_moving_copying(R"(abra\nka\ndabra\nka)", R"(\n)")
	);

	word_tokenizer_usage(
		word_tokenizer_moving_copying(LR"(abra\nka\ndabra\nka)", LR"(\n)")
	);
}

DBJ_TEST_SPACE_CLOSE
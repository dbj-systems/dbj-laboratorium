// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// testing the various stuff in this project
#include "pch.h"

#include "dbj_ctstring.h"
#include "dbj_lambda_lists.h"
// #include "dbj_tokenizer.h"
#include "dbj_atoms.h"
#include "dbj_lambda_lists.h"
#include "dbj_any_node.h"
#include "dbj_tree_tests.h"

#include "string_literals_with_universal_character_names.h"
#include "no_copy_no_move.h"

DBJ_TEST_SPACE_OPEN(local_tests)

/**************************************************************************************************/
using namespace std::string_view_literals;

struct STANDARD {
	constexpr static const auto compiletime_static_string_view_constant()
	{
		constexpr static auto
			make_once_and_only_if_called
			= "constexpr string view literal"sv;
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

	auto DBJ_MAYBE(where) = the_constant.find('e');
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
constexpr auto try_array(T *)  -> size_t
{
	return  std::extent_v< T >;
}

template<typename T>
constexpr auto probe_array(T &&)   -> size_t {
	return  std::extent_v< T >;
}

DBJ_TEST_UNIT(_array_stays_array)
{
	static int ia[]{ 1,2,3,4,5,6,7,8,9,0 };
	DBJ_TEST_ATOM(try_array(ia));
	DBJ_TEST_ATOM(DBJ_IS_ARR(ia));
	DBJ_TEST_ATOM(probe_array(ia));
	DBJ_TEST_ATOM(probe_array(std::addressof(ia)));
}

typedef enum class CODE : UINT {
	page_1252 = 1252u,   // western european windows
	page_65001 = 65001u // utf8
	// page_1200 = 1200,  // utf16?
	// page_1201 = 1201   // utf16 big endian?
} CODE_PAGE;

#if 0
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
	auto DBJ_MAYBE(fwp_rezult) = fwprintf(stdout, L"\nwfprintf() displays: %s\n", specimen);
	// for any mode the following crashes the UCRT (aka Universal CRT)
	// fprintf( stdout, "\nprintf() result: %S\n",specimen);
}
#endif

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

DBJ_TEST_UNIT(util_to_remove_duplicates)
{
	using dbj::util::remove_duplicates;
	int ia[10]{ 0,8,3,4,6,6,7,8,7,1 };

	DBJ_ATOM_TEST(ia);
	auto smart_pair_
		= remove_duplicates(ia, ia + 10, true); // sorted too

	DBJ_ATOM_TEST(smart_pair_);
}

// for C++11
namespace cpp11
{
	// https://wandbox.org/permlink/WJ1tRs2NU2lxDAIi
	using namespace std;

	template<bool _Test, class T = void>
	using enable_if_t = typename enable_if<_Test, T>::type;

	template<class T>
	using remove_extent_t = typename remove_extent<T>::type;

	template<class T,
		class... A,
		enable_if_t<!is_array<T>::value, int> = 0>
		inline unique_ptr<T> make_unique(A&&... args_)
	{	// make a unique_ptr
		return (unique_ptr<T>(new T(forward<A>(args_)...)));
	}

	template<class T,
		enable_if_t<is_array<T>::value && extent<T>::value == 0, int> = 0>
		inline unique_ptr<T> make_unique(size_t size_)
	{	// make a unique_ptr
		typedef remove_extent_t<T> E;
		return (unique_ptr<T>(new E[size_]()));
	}
}

/*
http://www.strudel.org.uk/itoa/
*/
namespace lukas_chmela {
	/**
	 * C++ version 0.4 char* style "itoa":
	 * Written by Lukás Chmela
	 * Released under GPLv3.
	 */
	char* itoa(int value, char* result, int base) {
		// check that the base if valid
		if (base < 2 || base > 36) { *result = '\0'; return result; }

		char* ptr = result, *ptr1 = result, tmp_char;
		int tmp_value;

		do {
			tmp_value = value;
			value /= base;
			*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
		} while (value);

		// Apply negative sign
		if (tmp_value < 0) *ptr++ = '-';
		*ptr-- = '\0';
		while (ptr1 < ptr) {
			tmp_char = *ptr;
			*ptr-- = *ptr1;
			*ptr1++ = tmp_char;
		}
		return result;
	}
}

namespace strudel {

	std::string itoa_original(int value, int base) {

		std::string buf;

		// check that the base if valid
		if (base < 2 || base > 16) return buf;

		enum { kMaxDigits = 35 };
		buf.reserve(kMaxDigits); // Pre-allocate enough space.


		int quotient = value;

		// Translating number to string with base:
		do {
			buf += "0123456789abcdef"[std::abs(quotient % base)];
			quotient /= base;
		} while (quotient);

		// Append the negative sign
		if (value < 0) buf += '-';

		std::reverse(buf.begin(), buf.end());
		return buf;
	}

	/*
	 Taken this http://www.strudel.org.uk/itoa/#newest
	 and made it work with unique_ptr<char[]>

	 note: this is faster than string and vector<char>

	 dbj 2019-02-06
	 */
	using char_buf = std::unique_ptr<char[]>;

	char_buf itoa(int value, int base) {

		constexpr auto kMaxDigits = 35U;
		char_buf buf;

		// check that the base if valid
		if (base < 2 || base > 16) return buf;

		buf = std::make_unique<char[]>(kMaxDigits + 1); // Pre-allocate enough space.

		int quotient = value;
		unsigned idx = 0;
		// Translating number to string with base:
		do {
			buf[idx++] = "0123456789abcdef"[std::abs(quotient % base)];
			quotient /= base;
		} while (quotient);

		// Append the negative sign
		if (value < 0) buf[idx++] = '-';

		// zero limit -- perhaps not necessary
		buf[idx] = char(0);

		std::reverse(buf.get(), buf.get() + idx);
		return buf;
	}


	std::array<char,35> itoa_array(int value, int base) 
	{

		constexpr auto kMaxDigits = 35U;
		std::array<char, 35>  buf{ {0} };

		// check that the base if valid
		if (base < 2 || base > 16) return buf;

		int quotient = value;
		unsigned idx = 0;
		// Translating number to string with base:
		do {
			buf[idx++] = "0123456789abcdef"[std::abs(quotient % base)];
			quotient /= base;
		} while (quotient);

		// Append the negative sign
		if (value < 0) buf[idx++] = '-';

		// zero limit -- perhaps not necessary
		buf[idx] = char(0);

		std::reverse(buf.data(), buf.data() + idx);
		return buf;
	}


	std::vector<char> itoa_vector(int value, int base)
	{

		constexpr auto kMaxDigits = 35U;
		std::vector<char>  buf;

		// check that the base if valid
		if (base < 2 || base > 16) return buf;

		buf = std::vector<char>();
		buf.resize(kMaxDigits); // Pre-allocate enough space.
		buf.reserve(kMaxDigits + kMaxDigits); // Pre-allocate enough space.

		int quotient = value;
		unsigned idx = 0;
		// Translating number to string with base:
		do {
			buf[idx++] = "0123456789abcdef"[std::abs(quotient % base)];
			quotient /= base;
		} while (quotient);

		// Append the negative sign
		if (value < 0) buf[idx++] = '-';

		// zero limit -- perhaps not necessary
		buf[idx] = char(0);

		std::reverse(buf.data(), buf.data() + idx);
		return buf;
	}

}



DBJ_TEST_UNIT(array_whatever)
{
	using smarty = std::unique_ptr<char[]>;
	auto sp_ = cpp11::make_unique<char[]>(9 + 1);

	auto lambada = [](auto left_, auto right_)
	{
		static int c = 0xFF;
		static char buf[0xFF]{};
		std::generate(left_, right_, [&] {
			return dbj::util::itox::itod(c++);
		});
	};

	lambada(sp_.get(), sp_.get() + 9);
	DBJ_ATOM_TEST(sp_);

	DBJ_ATOM_TEST(strudel::itoa(255, 10));
}

DBJ_TEST_UNIT(measure_strudel_itoa)
{
	const auto iterations_count = 0xFFFF;
	const auto integer_to_transform = 0xFFFF;

	auto measure_strudel_original = [&]() {
		for (int k{}; k < iterations_count; k++)
			(void)strudel::itoa_original(integer_to_transform, 10);
	};
	auto measure_strudel_dbj = [&]() {
		for (int k{}; k < iterations_count; k++)
			(void)strudel::itoa(integer_to_transform, 10);
	};
	auto measure_strudel_vector = [&]() {
		for (int k{}; k < iterations_count; k++)
			(void)strudel::itoa_vector(integer_to_transform, 10);
	};
	auto measure_strudel_array = [&]() {
		for (int k{}; k < iterations_count; k++)
			(void)strudel::itoa_array(integer_to_transform, 10);
	};

	auto report = [&](auto title, auto fun_ ) 
	{

		using dbj::fmt::print;
		print("\nalgorithm: %s,\n\titerations: %d,\tresult: %s",
			title,
			iterations_count,
			dbj::kalends::miliseconds_measure(fun_)
		);
	};

	report(	"strudel_original",measure_strudel_original);
	report(	"strudel_dbj", measure_strudel_dbj);
	report(	"strudel_vector", measure_strudel_vector);
	report(	"strudel_array", measure_strudel_array);

}

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

char whatever(unsigned k) { return char(k); } char(*FP)(unsigned) = whatever;

char whenever(unsigned ) { return '*' ; } char(*P1F)(unsigned)  = whenever;

char whoever(unsigned ) { return 'A'; } typedef char(*P2F)(unsigned) ;

static_assert(std::is_invocable_v<P2F()>);

std::string char_to_int(std::string_view c) { return { c.data() }; }

template<P2F fun_>
auto call_fp( unsigned K = 65) {

	static_assert(!std::is_reference_v<P2F>);
	return fun_(K);
}

DBJ_TEST_UNIT(most_curious)
{
	{
	  auto A1 = call_fp<whatever>();
	  // works but it should not --
	  auto A2 = call_fp<P2F(42)>();
	}
	// cuiruous-ity
	auto lambada = []() -> bool {return true; };
		DBJ_TEST_ATOM( typeid(lambada).name() );

	auto lambada2 = std::function<bool()>();
		DBJ_TEST_ATOM(typeid(lambada2).name());

	DBJ_USD( whatever(65)); // 'A'
	DBJ_USD( FP(66)); // 'B'
	DBJ_USD( whenever(66)) ; // '6

	

	// the interesting part
	// P2F is a type
	// and it has a default constructor
	// the returns instance of that type
	// which is function pointer
	DBJ_USD(whoever(42)); // '*'
	{

		//char(*P2F)(unsigned)
		using really_p2f = dbj::function_pointer<P2F>;

		DBJ_ATOM_TEST( typeid(really_p2f::empty_type).name() );

		{
			auto fun = really_p2f::is_callable<whenever>(65);
			auto A = really_p2f::applicator(whenever, 65);
		}

		{
			auto cheat = P2F(42);
			// auto fun = really_p2f::is_callable<cheat>();
			auto fun = really_p2f::is_callable<P2F(42)>(65);
			// auto A = really_p2f::applicator(cheat, 65);
		}

		// does not compile -- static_assert(std::is_invocable_v<(P2F)whenever>);
		using empty = std::invoke_result_t<P2F()>;
		DBJ_TEST_ATOM(typeid(empty).name()); //
		// does not compile -- static_assert(std::is_invocable_v<P2F(whenever)>);
		// does not compile -- static_assert(std::is_invocable_v<P2F(decltype(whenever)>);
		// does compile
		auto ww5 = static_cast<P2F>(whenever);
			DBJ_TEST_ATOM(ww5(65)); // call whoever
		auto ww4 = static_cast<P2F>(whatever);
			DBJ_TEST_ATOM(ww4(65)); // call whoever
		auto ww3 = static_cast<P2F>(whoever);
			DBJ_TEST_ATOM(ww3(65)); // call whoever
		
		// does not compile -- auto ww2 = static_cast<P2F>(char_to_int);
		// does not compile -- static_assert(std::is_invocable_v<P2F(char_to_int)>);
		// DBJ_TEST_ATOM(ww2(65)); // call whoever

		auto ww = P2F(whatever);
		DBJ_TEST_ATOM(ww);
		DBJ_TEST_ATOM(ww(65)); // calls whoever
	}
}
#endif // DBJ_MSVC_FIXED_FP_CASTING_FROM_ANYTHING

DBJ_TEST_SPACE_CLOSE

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// testing the various stuff in this project
#include "pch.h"

#include "dbj_ctstring.h"
#include "dbj_lambda_lists.h"
#include "dbj_tokenizer.h"
#include "dbj_atoms.h"
#include "dbj_lambda_lists.h"
#include "dbj_any_node.h"
#include "dbj_tree_tests.h"

#include "string_literals_with_universal_character_names.h"
#include "no_copy_no_move.h"

DBJ_TEST_SPACE_OPEN(local_tests)

/**************************************************************************************************/
using namespace std::string_view_literals;

#define  DBJ_LIGHT_BUFFER
#ifdef DBJ_LIGHT_BUFFER

/*
This is a runtime buffer. Just like std::vector, but better because it
is lighter and faster.
I knew it is a good practice to have and use one's own char buffer class
instead of using std::vector<char>
Here are the measurements too, in case you are hotly against:
https://stackoverflow.com/a/37709212/10870835

also this class works in code where no exceptions are used
please do let me know if problems there
*/
struct char_buffer final {

	using type = char_buffer;
	using value_type = char;
	using iterator = value_type * ;
	using pointer = std::unique_ptr<value_type[]>;
	/*
	one can "copy" the unique_ptr by moving, example:
	char_buffer<char> buf2 = std::move(buf1) ;
	this leaves the buf1 "destroyed" ... no good this is
	so how to pass them to functions?
	instead, for passing instances of this class as function arg's
	use this reference_type

	extern void whatever ( char_buffer<char>::reference_type br ) ;
	char_buffer<char> bf(BUFSIZ) ;
	// normal and simple calling
	whatever(bf);
	*/
	using reference_type = std::reference_wrapper<char_buffer>;

	explicit char_buffer(size_t size)
		// reference counted pointer to auto-delete the buffer
		: data_(std::make_unique<value_type[]>(size + 1)), size_(size)
	{
	}

	// result of this operator to change the single char
	char & operator [] (size_t idx_)
	{
		if (idx_ > size())
			throw std::make_error_code(std::errc::invalid_argument);
		return data_.get()[idx_];
	}

	iterator data() const { return data_.get(); }

	size_t const & size() const { return size_; }

	void reset() const {
		data_.reset('\0');
		data_ = std::make_unique<char[]>(size_ + 1);
	}

	value_type ** const address() const noexcept {
		auto p = std::addressof(data_.get()[0]);
		return std::addressof(p);
	}

	iterator begin() { return data_.get(); }
	iterator end() { return data_.get() + size_; }

	/*
	copy this one to the buffer of the same size
	return error_code(std::errc::invalid_argument)
	on different sizes
	return OK error_code on no error
	*/
	[[nodiscard]] friend
		std::error_code copy(const reference_type from_, reference_type target_)
		noexcept /* beware: std::copy may throw bad_alloc! */
	{
		auto const & frm_ = from_.get();
		auto & to_ = target_.get();

		if (frm_.size() != to_.size())
			return std::make_error_code(std::errc::invalid_argument);

		std::copy(frm_.data_.get(), frm_.data_.get() + frm_.size(),
			to_.data_.get());

		return std::error_code{};
	}

	using wide_pointer = std::unique_ptr<wchar_t[]>;
	/* in order not to loose the size info we will return this structure */
	struct wide_copy_result {
		std::error_code ec{};
		size_t size{};
		wide_pointer pointer{};
	};
	/*
	for the inevitable WIN32 aficionados
	we will deliver the wide copy of internal array,
	transformed and inside the unique_ptr inside the wide_copy_result
	*/
	static wide_copy_result wide_copy(reference_type cb) noexcept
	{
		auto const & source_ = cb.get();
		auto const & source_size_ = source_.size_;
		auto & source_pointer_ = source_.data_;

		wide_pointer wp = std::make_unique<wchar_t[]>(source_size_ + 1);
		size_t rezult_size;
		auto mbstowcs_errno = ::mbstowcs_s(
			&rezult_size,
			wp.get(), source_size_ + 1,
			source_pointer_.get(), source_size_
		);
		/*
		for (int j = 0; j < source_size_; j++) {
			 auto rez = std::mbtowc(
				& (wp.get())[j],
				& (source_pointer_.get())[j],
				1
			);
		}
		*/
		std::error_code ec{};
		if (0 != mbstowcs_errno) {
			ec = std::make_error_code(std::errc::invalid_argument);
		}
		return { ec, source_size_ , std::move(wp) };
	}

	// to avoid "never releasing smart pointer" syndrome
	// we wil ban creation of this class on the heap
	void* operator new(std::size_t sz) = delete;
	void* operator new[](std::size_t sz) = delete;
	void  operator delete(void* ptr, std::size_t sz) = delete;
	void  operator delete[](void* ptr, std::size_t sz) = delete;
private:
	mutable size_t  size_;
	mutable	pointer data_{};
};

DBJ_TEST_UNIT(dbj_light_buffer)
{
	using char_buff_type = char_buffer /*<char>*/;
	char_buff_type cb1(26), cb2(26);

	// test the heap (de)allocation
	// no can do --> auto p = new char_buff_type(BUFSIZ);
	// no can do --> auto p = new char_buff_type(255)[BUFSIZ];
	// global new and delete work as before o
	{
		auto p = new char[BUFSIZ];
		delete[] p;
	}
	// can we cheat with placement new?
	// no we can not
	//	auto p = new char_buff_type(BUFSIZ) char_buff_type;

	auto sizeshow = [&](char_buff_type::reference_type cbr)
	{
		std::vector<char> v26{}; v26.resize(26);
		std::string       s26{}; s26.resize(26);
		std::array<char, 26> a26{ {0} };

		auto retval = DBJ_ATOM_TEST(cbr.get().size());
		DBJ_ATOM_TEST(sizeof(a26));
		DBJ_ATOM_TEST(sizeof(v26));
		DBJ_ATOM_TEST(sizeof(s26));
		return retval;
	};

	// auto j = cb.size();
	auto a1 = cb1.address();

	auto alphabet = [&](char_buffer & cb) {
		char k = 65; // 'A'
		for (auto & c_ : cb)
		{
			c_ = char(k++);
			// char control = char(65 + j--);
		}
	};

	alphabet(cb1);
	DBJ_TEST_ATOM(sizeshow(cb1));

	auto ec_ = copy(cb1, cb2);

	if (ec_)
		DBJ_TEST_ATOM(ec_); // error
	else
		DBJ_TEST_ATOM(cb2.data()); // no error

/* test the wchar_t[] fetching, type returned is
struct char_buff_type::wide_copy_result {
	size_t size ;
	char_buff_type::wide_pointer pointer ;
}
*/
	{
		auto[err_code, wsize, wbuf] = char_buff_type::wide_copy(cb2);
		DBJ_ATOM_TEST(cb2.size());
		DBJ_ATOM_TEST(wsize);
		// 
		// wbuf is moved on return from wide_copy
		// no need to worry
		typename char_buff_type::wide_pointer & wide_buffer = wbuf;
		if (!err_code)
			DBJ_ATOM_TEST(wide_buffer.get());
		else
			::dbj::console::print("\nerror code returned: ", err_code);
	}
}

namespace inner {

	//deliberately not constexpr
	inline auto const & buffer_size = BUFSIZ;// 4000000;

	inline void naked_unique_ptr( size_t count_ ) {
		auto alloc_return = [&]() {
			return std::make_unique<char[]>(count_);
		};
		auto dumsy = alloc_return();
		dumsy.get()[count_ - 1] = '?'; 
	}

	inline void dbj_buffer(size_t count_) {
		auto alloc_return = [&]() {
			return char_buffer(count_);
		};
		auto dumsy[[maybe_unused]] = alloc_return();
		dumsy[count_ - 1] = '?';
	}

	inline auto allocateWithVector(size_t count_) {
		auto alloc_return = [&]() {
			return std::vector<unsigned char>(count_);
		};
		auto dumsy[[maybe_unused]] = alloc_return();
		dumsy[count_ - 1] = '?';
	}

	inline auto measure(
		void(*fp)(size_t), 
		size_t buffer_count, 
		size_t max_iteration = 1000) 
   {
		auto start_ = std::chrono::system_clock::now();
		for (long i = 0; i < max_iteration; i++) {
			fp(buffer_count); 
		}
		auto end_ = std::chrono::system_clock::now();
		return (end_ - start_).count() / 1000.0;
	}
} // inner

DBJ_TEST_UNIT(dbj_light_buffer_compare_to_vector) {
	/*
	on this machine I have found, for normal buffer sizes 
	std::vector<char> is considerably slower
	for very large buffers it is equaly slow as the two others
	*/
	using ::dbj::console::print;
	using namespace inner;
	system("@echo.");
	system("@echo.");
	print("\n will allocate and measure three types of buffers. buffer size will be ",
		buffer_size, " chars each\n\teach allocation/deallocation will happen 1000 times");

	print("\n\nMeasuring unique_ptr<char[]> ");
	print("\n\nunique_ptr<char[]> = ", measure(naked_unique_ptr, buffer_size), " ms.\n");
	print("\n\nMeasuring dbj char buffer ");
	print("\n\ndbj char buffer = ", measure(dbj_buffer, buffer_size), " ms.\n");
	print("\n\nMeasuring std::vector ");
	print("\n\nstd::vector = ", measure(allocateWithVector, buffer_size), " ms.\n\n");

	system("@echo.");
	system("@echo.");
	system("@pause");
	system("@echo.");
	system("@echo.");
}


#endif // DBJ_LIGHT_BUFFER


struct STANDARD {
	constexpr static const auto compiletime_static_string_view_constant()
	{
		constexpr static auto
			make_once_and_only_if_called = "constexpr string view literal"sv;
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

DBJ_TEST_SPACE_CLOSE

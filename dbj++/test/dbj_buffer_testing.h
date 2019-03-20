#pragma once

#define DBJ_LIGHT_BUFFER_TESTING
#ifdef DBJ_LIGHT_BUFFER_TESTING

#include "../console/dbj_console_ops.h"

#include "../test/zAllocator.h"


DBJ_TEST_SPACE_OPEN(dbj_buffer)

using namespace ::dbj::buf;

auto alphabet = [&]( buffer::reference_type cbr ) {
	char k = 65; // 'A'
	for (auto & c_ : cbr )
	{
		c_ = char(k++);
	}
};

extern "C"	inline void	my_memset(void *s, size_t n, char val_ = 0) noexcept
{
	volatile char *p = (char *)s;
	while (n--) *p++ = val_;
}

DBJ_TEST_UNIT(dbj_light_buffer)
{
	const auto bufsiz_ = BUFSIZ;
	buffer cb1(BUFSIZ);

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

	auto sizeshow = [&](buffer::reference_type cbr) noexcept -> void
	{
		auto show = [](auto && obj_, auto filler ) {
			filler(obj_);
			dbj::console::prinf("\ntype: %s\n\tsize of type: %zu\n\tsize of instance: %zu",
				typeid(obj_).name(), sizeof(decltype(obj_)), sizeof(obj_)
			);
		};

		show(std::vector<char>(BUFSIZ), [](auto & obj_) { std::fill(obj_.begin(), obj_.end(), '\0'); });
		show( std::string(BUFSIZ,'\0'), [](auto & obj_) { std::fill(obj_.begin(), obj_.end(), '\0'); });
		show(cbr, [](auto & obj_) { obj_.fill('*'); });
	};

	DBJ_TEST_ATOM(cb1.address());

	alphabet(cb1);	sizeshow(cb1);
	
	// show the copying
	{
		buffer cb2(BUFSIZ);
		DBJ_ATOM_TEST(cb2); //rezult
		DBJ_TEST_ATOM(cb1 = cb2); // assignment

		
		assert(cb1.data());
		assert(cb2.data());

		assert(cb1.size() == cb2.size());
	}
// tranformations to string, wstring and vector
	{
		cb1.fill('X'); // assignment
		DBJ_ATOM_TEST(cb1);
		DBJ_ATOM_TEST(to_string(cb1));
		DBJ_ATOM_TEST(to_vector(cb1));
	}

	{
		auto[err_code, wsize, smart_wide_charr] = wide_copy(cb1);
		DBJ_ATOM_TEST(err_code);
		DBJ_ATOM_TEST(wsize);
		DBJ_ATOM_TEST(smart_wide_charr);
	}
}

namespace inner {

	//deliberately not constexpr
	inline auto const & buffer_size = ::dbj::buf::max_length ;
	inline auto const & max_iterations = 1000;

	inline std::unique_ptr<char[]> naked_unique_ptr(size_t count_) {
			return std::make_unique<char[]>(count_);
	}

	inline buffer dbj_buffer(size_t count_) {
			return buffer(count_);
	}

	inline std::vector<char> vector_buffer(size_t count_) {
			return std::vector<char>(count_);
	}

	inline std::string string_buffer(size_t count_) {
		return std::string(size_t(count_), char(0));
	}

	using char_allocator = esapi::zallocator<char>;
	using zvector = std::vector< char, char_allocator >;
	using zstring = std::basic_string< char, std::char_traits<char>, char_allocator >;

	inline zvector 
		vector_buffer_zallocator(size_t count_) {
			return zvector(count_);
	}

	inline zstring
		string_buffer_zallocator(size_t count_) {
			return zstring( size_t(count_), char(0) );
	}

	/*
	measure the performance of making/destroying three kinds of buffers
	dbj buf, dbj buffer and vector<char>
	size of the buffers is user defined 
	*/
	auto measure = [] (
		auto fp ,
		::dbj::buf::inside_1_and_max buffer_count,
		size_t max_iteration = max_iterations )
	{
		auto start_ = std::chrono::system_clock::now();
		for (long i = 0; i < max_iteration; i++) {
			auto dumsy = fp(buffer_count);
			dumsy[buffer_count - 1] = '?';
		}
		auto end_ = std::chrono::system_clock::now();
		return (end_ - start_).count() / 1000.0;
	};
} // inner

DBJ_TEST_UNIT(dbj_light_buffer_measure) {
	/*
	on this machine I have found, for normal buffer sizes
	std::vector<char> is considerably slower
	for very large buffers it is equaly slow as the two others
	*/
	using ::dbj::console::print;
	using namespace inner;
	print("\nWill make/destroy on the stack, and measure FOUR types of buffers. Buffer size will be ",
		buffer_size, " chars each\n\tEach allocation/deallocation will happen ",
		max_iterations, " times\n\n");

	print( measure(naked_unique_ptr, buffer_size), " ms. \tunique_ptr<char[]>\n");
	print( measure(dbj_buffer, buffer_size), " ms. \tdbj char buffer\n");
	print( measure(vector_buffer, buffer_size), " ms. \tstd::vector\n" );
	// print( measure(vector_buffer_zallocator, buffer_size), " ms. \tzallocator + std::vector\n");
	print( measure(string_buffer, buffer_size), " ms. \tstd::string\n");
	// print( measure(string_buffer_zallocator, buffer_size), " ms. \tzallocator + std::string\n\n");

	// extremely slow with zAllocator
	
	system("@pause");
	system("@echo.");
}

DBJ_TEST_SPACE_CLOSE

#undef _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING

#endif // DBJ_LIGHT_BUFFER_TESTING


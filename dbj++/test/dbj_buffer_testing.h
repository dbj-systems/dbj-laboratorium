#pragma once

#define DBJ_LIGHT_BUFFER_TESTING
#ifdef DBJ_LIGHT_BUFFER_TESTING

DBJ_TEST_SPACE_OPEN(dbj_buffer)

using namespace ::dbj::chr_buf;
using namespace ::dbj::chr_buffer;

#pragma region some yanb helper testing 

DBJ_TEST_UNIT(some_yanb_helper_testing) {

	namespace buf = ::dbj::chr_buf;

	auto test_1 = [](auto C_, auto specimen)
	{
		using T = ::std::decay_t< decltype(C_) >;
		using helper = buf::yanb_helper<T>;

		DBJ_TUNIT(yanb_tpl<T>(BUFSIZ));
		DBJ_TUNIT(helper::make(specimen));
		DBJ_TUNIT(helper::make(std::basic_string<T>(specimen)));
		DBJ_TUNIT(helper::make(std::basic_string_view<T>(specimen)));

		auto buf_ = yanb_tpl<T>(BUFSIZ);
		DBJ_TUNIT(helper::fill(buf_, C_));

		auto sec_ = helper::make(buf_);
		DBJ_TUNIT(sec_ == buf_);

	};
	test_1('*', "narrow string");
	test_1(L'*', L"wide string");
}

#pragma endregion

DBJ_TEST_UNIT(dbj_light_buffer)
{
	const auto bufsiz_ = BUFSIZ;

	auto alphabet = [&](buffer::reference_type cbr) {
		char k = 65; // 'A'
		for (auto& c_ : cbr)
		{
			c_ = char(k++);
		}
	};

	auto my_memset = [](void* s, size_t n, char val_ = 0) noexcept
	{
		volatile char* p = (char*)s;
		while (n--)* p++ = val_;
	};

	auto sizeshow = [&](buffer::reference_type cbr) noexcept -> void
	{
		auto show = [](auto&& obj_, auto filler) {
			filler(obj_);
			dbj::console::prinf("\ntype: %s\n\tsize of type: %zu\n\tsize of instance: %zu",
				typeid(obj_).name(), sizeof(decltype(obj_)), sizeof(obj_)
			);
		};

		show(std::vector<char>(BUFSIZ), [](auto& obj_) { std::fill(obj_.begin(), obj_.end(), '\0'); });
		show(std::string(BUFSIZ, '\0'), [](auto& obj_) { std::fill(obj_.begin(), obj_.end(), '\0'); });
		show(cbr, [](auto& obj_) { obj_.fill('*'); });
	};

	buffer cb1(BUFSIZ);

	alphabet(cb1);	sizeshow(cb1);

	// show the copying
	{
		buffer cb2(BUFSIZ);
		DBJ_ATOM_TEST(cb2); //rezult
		DBJ_TEST_ATOM(cb1 = cb2); // assignment

		DBJ_VERIFY(cb1.data());
		DBJ_VERIFY(cb2.data());

		DBJ_VERIFY(cb1.size() == cb2.size());
	}
	// tranformations to string, wstring and vector
	{
		cb1.fill('X'); // assignment
		DBJ_ATOM_TEST(cb1);
		DBJ_ATOM_TEST(to_string(cb1));
		DBJ_ATOM_TEST(to_vector(cb1));
	}

	{
		auto [err_code, wsize, smart_wide_charr] = wide_copy(cb1);
		DBJ_ATOM_TEST(err_code);
		DBJ_ATOM_TEST(wsize);
		DBJ_ATOM_TEST(smart_wide_charr);
	}
}

namespace inner {

	//deliberately not constexpr
	inline auto const& buffer_size = ::dbj::chr_buf::DBJ_MAX_BUFF_LENGTH;
	inline auto const& max_iterations = 1000;

	inline std::unique_ptr<char[]> naked_unique_ptr(size_t count_)
	{
		return std::make_unique<char[]>(count_ + 1);
	}

	inline std::shared_ptr<char[]> naked_shared_ptr(size_t count_)
	{
		return std::shared_ptr<char[]>( new char[count_+1]);
	}

	inline ::dbj::chr_buf::yanb dbj_yanb(size_t count_) {
		return ::dbj::chr_buf::yanb(count_);
	}

	inline auto dbj_vector_buffer(size_t count_) {
		return  dbj::vector_buffer<char>::make(count_);
	}

	inline std::string string_buffer(size_t count_) {
		return std::string (static_cast<size_t>(count_), static_cast<char>(0));
	}

	/*
	measure the performance of making/destroying three kinds of buffers
	dbj buf, dbj buffer and vector<char>
	size of the buffers is user defined
	*/
	auto measure = [](
		auto fp,
		::dbj::chr_buf::between_0_and_max buffer_sz,
		size_t max_iteration = max_iterations)
	{
		auto start_ = std::chrono::system_clock::now();
		for (long i = 0; i < max_iteration; i++)
		{
			// create the buffer
			// then move it on return from fp()
			auto dumsy = fp(buffer_sz);
			// call the '[]' operator 
			// and change the char value
			// for each char
			for (::dbj::chr_buf::between_0_and_max j = 0; j < buffer_sz; j++ )
					dumsy[j] = '?';
		}
		auto end_ = std::chrono::system_clock::now();
		const auto micro_seconds = (end_ - start_).count() / 1000.0;
		return  micro_seconds / 1000; // milli seconds
	};
} // inner

DBJ_TEST_UNIT(dbj_buffers_comparison) {

	auto & print = ::dbj::console::print;

	print("\n\nWill make/destroy on the stack, and measure five types of buffers\n\n");
	/*
	on this machine I have found, for normal buffer sizes
	std::vector<char> is considerably slower
	for very large buffers it is equaly slow as the two others
	*/

	auto driver = [&](auto  buf_size_) 
	{
		print("\n\nBuffer size:\t\t",	buf_size_, "\nIterations:\t\t",max_iterations,"\n\n");
		print(measure(naked_unique_ptr, buf_size_), " miki s. \tunique_ptr<char[]>\n");
		print(measure(naked_shared_ptr, buf_size_), " miki s. \tshared_ptr<char[]>\n");
		print(measure(dbj_yanb,			buf_size_), " miki s. \tdbj YANB buffer\n");
		print(measure(dbj_vector_buffer,buf_size_), " miki s. \tstd::vector\n");
		print(measure(string_buffer,	buf_size_), " miki s. \tstd::string\n");
	};

	using namespace inner;
	driver(buffer_size / 1  );
	driver(buffer_size / 2  );
	driver(buffer_size / 4  );
	driver(buffer_size / 8  );
	driver(buffer_size / 16 );

	system("@pause");
	system("@echo.");
}

DBJ_TEST_SPACE_CLOSE

#undef _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING

#endif // DBJ_LIGHT_BUFFER_TESTING


#pragma once

#define DBJ_LIGHT_BUFFER_TESTING
#ifdef DBJ_LIGHT_BUFFER_TESTING

DBJ_TEST_SPACE_OPEN(dbj_buffer)


DBJ_TEST_UNIT(some_yanb_helper_testing) {

	auto driver = [](auto C_, auto specimen)
	{
		using T = ::std::decay_t< decltype(C_) >;
		using helper = dbj::vector_buffer<T>;

		DBJ_TUNIT(helper::make(BUFSIZ));
		DBJ_TUNIT(helper::make(specimen));
		DBJ_TUNIT(helper::make(std::basic_string<T>(specimen)));
		DBJ_TUNIT(helper::make(std::basic_string_view<T>(specimen)));

		auto buf_ = helper::make(BUFSIZ) ;
		auto sec_ = buf_ ;
		DBJ_TUNIT(sec_ == buf_);

	};
	driver('*', "narrow string");
	driver(L'*', L"wide string");
}


DBJ_TEST_UNIT(dbj_light_buffer)
{
	const auto bufsiz_ = BUFSIZ;
	using helper = dbj::vector_buffer<char>;

	auto alphabet = [&](helper::narrow & cbr)
	{
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

	auto sizeshow = [&]( helper::narrow cbr) noexcept -> void
	{
		auto show = [](auto&& obj_, auto filler) {
			filler(obj_);
			dbj::console::prinf("\ntype: %s\n\tsize of type: %zu\n\tsize of instance: %zu",
				typeid(obj_).name(), sizeof(decltype(obj_)), sizeof(obj_)
			);
		};

		show(std::vector<char>(BUFSIZ), [](auto& obj_) { std::fill(obj_.begin(), obj_.end(), '\0'); });
		show(std::string(BUFSIZ, '\0'), [](auto& obj_) { std::fill(obj_.begin(), obj_.end(), '\0'); });
		show(cbr, [](auto& obj_) {  std::fill(obj_.begin(), obj_.end(), '*'); } );
	};

	helper::narrow cb1(BUFSIZ);

	alphabet(cb1);	sizeshow(cb1);

	// show the copying
	{
		helper::narrow cb2(BUFSIZ);
		DBJ_ATOM_TEST(cb2); //rezult
		DBJ_TEST_ATOM(cb1 = cb2); // assignment

		DBJ_VERIFY(cb1.data());
		DBJ_VERIFY(cb2.data());

		DBJ_VERIFY(cb1.size() == cb2.size());
	}
	// tranformations to string, wstring and vector
	{
		auto filler = [](auto& obj_ , char fillchr ) {  std::fill(obj_.begin(), obj_.end(), fillchr); };
		filler( cb1, 'X'); // assignment
		DBJ_ATOM_TEST(cb1);
	}
}

namespace inner {

	//deliberately not constexpr
	inline auto const& buffer_size = ::dbj::DBJ_64KB;
	inline auto const& max_iterations = 1000;

	inline auto naked_unique_ptr(size_t count_)
	{
		return std::make_unique<char[]>(count_ + 1);
	}

	inline auto naked_shared_ptr(size_t count_)
	{
		return std::shared_ptr<char[]>( new char[count_+1]);
	}

	inline auto  uniq_ptr_buffer(size_t count_) {
		return dbj::unique_ptr_buffer<char>(count_);
	}

	inline auto dbj_vector_buffer(size_t count_) {
		return  dbj::vector_buffer<char>::make(count_);
	}

	inline auto string_buffer(size_t count_) {
		return std::string (static_cast<size_t>(count_), static_cast<char>(0));
	}

	inline auto string_view_buffer(size_t count_) 
	{
		// string view has no facility to create char buffer of certain size
		static auto up = std::make_unique<char[]>(count_ + 1);
		return std::string_view (up.get(), count_ );
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
			// using this nasty hack
			for ( unsigned j = 0; j < buffer_sz; j++ )
					 const_cast<char &>( dumsy[j]) = '?';
		}
		auto end_ = std::chrono::system_clock::now();
		const auto micro_seconds = (end_ - start_).count() / 1000.0;
		return  micro_seconds / 1000; // milli seconds
	};
} // inner

DBJ_TEST_UNIT(dbj_buffers_comparison) {

	using namespace inner;
	auto & print = ::dbj::console::print;

	print("\n\nWill test and measure six types of buffers\n\n");

	/*
	on this machine I have found, 
	std::string is considerably slower
	*/

	auto driver = [&](auto  buf_size_) 
	{
		print("\n\nBuffer size:\t\t",		buf_size_,	"\nIterations:\t\t",max_iterations,"\n\n");
		print(measure(naked_unique_ptr,		buf_size_), " miki s. \t unique_ptr<char[]>\n");
		print(measure(naked_shared_ptr,		buf_size_), " miki s. \t shared_ptr<char[]>\n");
		print(measure(uniq_ptr_buffer,		buf_size_), " miki s. \t	unique_ptr_buffer<char>\n");
		print(measure(dbj_vector_buffer,	buf_size_), " miki s. \t std::vector\n");
		print(measure(string_view_buffer,	buf_size_), " miki s. \t std::string_view\n");
		print(measure(string_buffer,		buf_size_), " miki s. \t std::string\n");
	};

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


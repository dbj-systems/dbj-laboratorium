#pragma once

#define DBJ_LIGHT_BUFFER_TESTING
#ifdef DBJ_LIGHT_BUFFER_TESTING

DBJ_TEST_SPACE_OPEN(dbj_buffer)



DBJ_TEST_UNIT(buffers_and_literals)
{
	// using namespace std;
	using namespace ::dbj::core::unvarnished ;
	using namespace ::dbj::unique_ptr_buffers;
	// auto & print = console::print ;

	DBJ_TEST_ATOM(::dbj::arr::string_literal_to_std_char_array("WOW?!"));

	// std::array{ '1','2','3' }
	DBJ_TEST_ATOM(123_std_char_array);
	// std::array{ '0','x','1','2' }
	DBJ_TEST_ATOM(0x12_std_char_array);
	// std::array{ '4'.'2','.','1','3' }
	DBJ_TEST_ATOM(42.13_std_char_array);

	DBJ_TEST_ATOM("STRING LITERAL"_buffer);
	DBJ_TEST_ATOM(L"WIDE STRING LITERAL"_buffer);

	DBJ_TEST_ATOM("STRING LITERAL"_buffer_pair);
	DBJ_TEST_ATOM(L"WIDE STRING LITERAL"_buffer_pair);

	DBJ_TEST_ATOM("STRING LITERAL"_v_buffer);
	DBJ_TEST_ATOM(L"WIDE STRING LITERAL"_v_buffer);

	// usability
	auto [ s_1, p_1 ]	= "STRING LITERAL"_buffer_pair ;
	auto [ s_2, p_2 ]	= L"WIDE STRING LITERAL"_buffer_pair ;

	DBJ_TEST_ATOM(up_buffer_make("Buffy"));

}



DBJ_TEST_UNIT(some_vector_buffer_helper_testing) {

	auto driver = [](auto C_, auto specimen)
	{
		using T = ::std::decay_t< decltype(C_) >;
		using helper = dbj::vector_buffer<T>;

		DBJ_TUNIT(helper::make(BUFSIZ));
		DBJ_TUNIT(helper::make(specimen));
		DBJ_TUNIT(helper::make(std::basic_string<T>(specimen)));
		DBJ_TUNIT(helper::make(std::basic_string_view<T>(specimen)));
	};
	driver('*', "narrow string");
	driver(L'*', L"wide string");
}


DBJ_TEST_UNIT(dbj_light_buffer)
{
	const auto bufsiz_ = BUFSIZ;
	using helper = dbj::vector_buffer<char>;

	auto driver = [&]( typename dbj::vector_buffer<char>::narrow cbr) noexcept -> void
	{
		auto alphabet = [](auto & cbr)
		{
			char k = 65; // 'A'
			for (auto& c_ : cbr)
			{
				c_ = char( (k++) % (65 + 26) );
			}
			return cbr;
		};

		alphabet(cbr);
		DBJ_TEST_ATOM( cbr );
		DBJ_ATOM_TEST( cbr.size() );
		DBJ_TEST_ATOM( cbr.data() );
		DBJ_ATOM_TEST( std::strlen(cbr.data() ) );
	};

	driver(helper::narrow(BUFSIZ));
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
		return dbj::unique_ptr_buffer_type<char>(count_);
	}

	inline auto dbj_vector_buffer(size_t count_) {
		return  dbj::vector_buffer<char>::make(count_);
	}

	inline auto string_buffer(size_t count_) {
		return std::string (static_cast<size_t>(count_), static_cast<char>(0));
	}

	inline auto string_view_buffer(size_t count_) 
	{
		return dbj::runtime_shared_string_view_buffer<char>( count_ );
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

	print("\n\nWill test and measure six types of runtime buffers\n\n");

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


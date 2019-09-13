#pragma once
#include <ctime>
#include <chrono>



TU_REGISTER([] {

	auto driver = [](auto C_, auto specimen)
	{
		using T = ::std::decay_t< decltype(C_) >;
		using helper = dbj::vector_buffer<T>;

		DBJ_TX(  helper::make(BUFSIZ) );
		DBJ_TX(  helper::make(specimen) );
		DBJ_TX(  helper::make(std::basic_string<T>(specimen)));
		DBJ_TX(  helper::make(std::basic_string_view<T>(specimen)));
	};
	driver('*', "narrow string");
	driver(L'*', L"wide string");
	});


TU_REGISTER([] {
	// not as fast as naked unique ptr
	// but comfortable api
	using buffer = dbj::unique_ptr_buffer_type<char>;

	auto mover = [](buffer b_) -> buffer { return b_; };

	DBJ_TX(buffer(0xFF).size());
	DBJ_TX(mover(buffer("ABC")));
	DBJ_TX(mover(buffer("ABC")).size());
	DBJ_TX(mover(buffer("ABC"))[0]);
	DBJ_TX(mover(buffer("ABC")).buffer());

	auto buffy = buffer("DEF");
	buffy[0] = 'D';
	buffy[1] = 'E';
	buffy[2] = 'F';
	DBJ_TX(buffy);
	});

TU_REGISTER([] {
	using namespace ::dbj::compile_time_buffers;

	DBJ_TX(narrow());
	DBJ_TX(narrow< 0xFF >());
	DBJ_TX(wide());
	DBJ_TX(wide< 0xFF >());

	});

TU_REGISTER([] {
	// using namespace std;
	using namespace ::dbj::core::unvarnished;
	using namespace ::dbj::unique_ptr_buffers;
	// auto & print = console::print ;

	// std::array{ '1','2','3' }
	DBJ_TX(123_std_char_array);
	// std::array{ '0','x','1','2' }
	DBJ_TX(0x12_std_char_array);
	// std::array{ '4'.'2','.','1','3' }
	DBJ_TX(42.13_std_char_array);

	DBJ_TX("STRING LITERAL"_buffer);
	DBJ_TX(L"WIDE STRING LITERAL"_buffer);

	DBJ_TX("STRING LITERAL"_buffer_pair);
	DBJ_TX(L"WIDE STRING LITERAL"_buffer_pair);

	DBJ_TX("STRING LITERAL"_v_buffer);
	DBJ_TX(L"WIDE STRING LITERAL"_v_buffer);

	// usability
	auto [s_1, p_1] = "STRING LITERAL"_buffer_pair;
	auto [s_2, p_2] = L"WIDE STRING LITERAL"_buffer_pair;

	DBJ_TX(up_buffer_make("Buffy"));

	});

TU_REGISTER([] {

	const auto bufsiz_ = BUFSIZ;
	using helper = dbj::vector_buffer<char>;

	auto driver = [&](typename dbj::vector_buffer<char>::buffer_type cbr) noexcept -> void
	{
		auto alphabet = [](auto& cbr)
		{
			char k = 65; // 'A'
			for (auto& c_ : cbr)
			{
				c_ = char((k++) % (65 + 26));
			}
			return cbr;
		};

		alphabet(cbr);
		DBJ_TX(cbr);
		DBJ_TX(cbr.size());
		DBJ_TX(cbr.data());
		DBJ_TX(std::strlen(cbr.data()));	};

	driver(helper::narrow(BUFSIZ));
	});

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

	/*
	measure the performance of making/destroying three kinds of buffers
	dbj buf, dbj buffer and vector<char>
	size of the buffers is user defined
	*/
	inline auto measure = [](
		auto fp,
		size_t buffer_sz,
		size_t max_iteration = max_iterations)
	{
		// good enough for the purpose
		auto random = [](int max_val, int min_val = 1)
			-> int {
			static auto initor = []() {
				std::srand((unsigned)std::time(nullptr));
				return 0;
			}();
			return ::abs(int(min_val + std::rand() / ((RAND_MAX + 1u) / max_val)));
		};

		auto start_ = std::chrono::system_clock::now();
		for (size_t i = 0; i < max_iteration; i++)
		{
			// create the buffer
			// then move it on return from fp()
			auto dumsy = fp(buffer_sz);
			// call the '[]' operator 
			// and change the char value
			for ( unsigned j = 0; j < buffer_sz ; j++ )
				    // prevent the compiler to optimize away the loop
					 dumsy[j] =  char( random( 64 + 25, 64) );
		}
		auto end_ = std::chrono::system_clock::now();
		const auto micro_seconds = (end_ - start_).count() / 1000.0;
		return  micro_seconds / 1000; // milli seconds
	};
} // inner



TU_REGISTER([] {

	using namespace inner;

	DBJ_PRINT(DBJ_BG_BLUE
		"\n\nTest and measure FIVE types of runtime buffers\n"  
		DBJ_RESET);
	DBJ_PRINT("\n\nThe test code, in a loop, creates each buffer type, then moves it and finally\n "
		"changes the buffer content by using the '[]' operator, char by char\n"
		"Care has been taken for compiler not to 'optimize out' the testing code\n"
	);
#ifdef _WIN64
	DBJ_PRINT("\n64 bit build\n");
#else
	DBJ_PRINT("\n32 bit build\n");
#endif // WIN64

	/*
	on this machine I have found,
	std::string is considerably slower than anything else
	the difference is more obvious on 32 bit builds vs 64 bit ones
	*/

	auto driver = [&](size_t  buf_size_)
	{
		DBJ_PRINT( "\nBuffer size: %zu \tIterations: %d\n\n", buf_size_, max_iterations );
		DBJ_PRINT( "1. %f miki s. \t unique_ptr<char[]>\n" , measure(naked_unique_ptr, buf_size_));
		DBJ_PRINT( "2. %f miki s. \t std::vector<char>\n", measure(dbj_vector_buffer, buf_size_));
		DBJ_PRINT( "3. %f miki s. \t dbj unique_ptr_buffer<char>\n", measure(uniq_ptr_buffer, buf_size_));
		DBJ_PRINT( "4. %f miki s. \t shared_ptr<char[]>\n", measure(naked_shared_ptr, buf_size_));
		DBJ_PRINT( "5. %f miki s. \t std::string\n", measure(string_buffer, buf_size_));
	};

	driver(buffer_size / 1);
	driver(buffer_size / 2);
	driver(buffer_size / 4);
	driver(buffer_size / 8);
	driver(buffer_size / 16);

	system("@pause");
	system("@echo.");
	});


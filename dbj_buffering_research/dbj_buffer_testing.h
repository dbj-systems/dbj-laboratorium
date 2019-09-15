#pragma once




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



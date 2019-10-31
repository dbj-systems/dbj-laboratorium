#pragma once

namespace measure_buffers_performance {
	namespace inner {

		using buffer_h = typename dbj::nanolib::v_buffer;
		using buffer_t = typename dbj::nanolib::v_buffer::buffer_type;

		inline auto const& buffer_size = ::dbj::DBJ_64KB;
		inline auto const& max_iterations = 1000;

		inline auto dbj_shared_ptr_buffer(size_t count_)
		{
			return dbj::shared_pointer_buffer<char>(count_ + 1);
		}

		inline auto naked_unique_ptr(size_t count_)
		{
			return std::make_unique<char[]>(count_ + 1);
		}

		inline auto naked_shared_ptr(size_t count_)
		{
			return std::shared_ptr<char[]>(new char[count_ + 1]);
		}

		inline auto  uniq_ptr_buffer(size_t count_) {
			return dbj::unique_ptr_buffer_type<char>(count_);
		}

		inline auto dbj_vector_buffer(size_t count_) {
			return  dbj::vector_buffer<char>::make(count_);
		}

		inline auto string_buffer(size_t count_) {
			return std::string(static_cast<size_t>(count_), static_cast<char>(0));
		}

		/*
		measure the performance of making/destroying three kinds of buffers
		dbj buf, dbj buffer and vector<char>
		size of the buffers is user defined
		*/
		inline auto measure = [](
			auto buffer_maker_f ,
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
				// create the buffer of required size
				// move it on return 
				auto the_buffer = buffer_maker_f(buffer_sz);
				for (unsigned j = 0; j < buffer_sz; j++)
					// call the '[]' operator 
					// change the char value
					// prevent the compiler to optimize away the loop
					the_buffer[j] = char(random(64 + 25, 64));
			}
			auto end_ = std::chrono::system_clock::now();
			const auto micro_seconds = (end_ - start_).count() / 1000.0;
			return  micro_seconds; //  / 1000; // milli seconds
		};

		using results_map = std::map< double, buffer_t >;

		inline results_map results{};

		inline auto store_result = [&](buffer_t fmt_, double time_)
		{
			results.emplace(time_, fmt_);
		};

		inline auto show_results = [&](bool clear = true)
		{
			using buf  = dbj::nanolib::v_buffer;
			using buf_t = typename buf::buffer_type;
			//
			buf_t buffy = buf::format("\n%3s,%6s,%6s,%8s,%s", "Ord", "iterations", "size", "rezult", "comment");
			int position = 1;
			for (auto [time_, fmt_] : results) {
				buffy = buf::format("%s\n%3d,", buffy.data(),  position++);
				buf_t buffy_2 = buf::format(fmt_.data(), time_);
				buffy = buf::format("%s%s", buffy.data(), buffy_2.data());
			}
			if (clear) results.clear();

			log_trace("%s", buffy.data());
		};

		// prepare the prompt
		// prompt is stored in the map and used at the moment of printing
		// in the show_results()
		inline auto prompt = [](size_t iterations_, size_t size_, const char* comment_)
			-> buffer_t
		{
			//std::printf("%6zd, %6zd, %%7.2f, %s", iterations_, size_, comment_);

			// iterations, size, result, comment
			// result coming after measurement
			return dbj::nanolib::v_buffer::format("%6zd, %6zd, %%7.2f, %s", iterations_, size_, comment_);
		};

		inline auto driver = [&](size_t  buf_size_)
		{
			log_trace("Buffer size: %zu \tIterations: %d", buf_size_, max_iterations);
			store_result(prompt(max_iterations, buf_size_, "unique_ptr<char[]>"), measure(naked_unique_ptr, buf_size_));
			store_result(prompt(max_iterations, buf_size_, "std::vector<char>"), measure(dbj_vector_buffer, buf_size_));
			store_result(prompt(max_iterations, buf_size_, "home made unique_ptr buffer"), measure(uniq_ptr_buffer, buf_size_));
			store_result(prompt(max_iterations, buf_size_, "shared_ptr<char[]>"), measure(naked_shared_ptr, buf_size_));
			store_result(prompt(max_iterations, buf_size_, "std::string"), measure(string_buffer, buf_size_));
			store_result(prompt(max_iterations, buf_size_, "home made shared_ptr buffer"), measure(dbj_shared_ptr_buffer, buf_size_));
		};
	} // inner namespace

	TU_REGISTER([] {

		using namespace inner;

		log_trace(
			"\n\nTest and measure several core types of runtime buffers\n"
			DBJ_FG_GREEN_BOLD
			"\n for (size_t i = 0; i < max_iteration; i++)"
			"\n {"
			DBJ_RESET DBJ_FG_GREEN
			"\n// create the buffer of required size"
			"\n// move it on return " 
			DBJ_RESET DBJ_FG_GREEN_BOLD
			"\n   auto the_buffer = buffer_maker_f(buffer_sz);"
			"\n   for (unsigned j = 0; j < buffer_sz; j++)"
			DBJ_RESET DBJ_FG_GREEN
			"\n	// call the '[]' operator "
			"\n	// change the char value"
			"\n	// prevent the compiler to optimize away" 
			DBJ_RESET DBJ_FG_GREEN_BOLD
			"\n	  the_buffer[j] = char(random(64 + 25, 64));"
			"\n  } }\n\n"
			DBJ_RESET
		);

#ifdef _WIN64
	#define CPU_TYPE "64 BIT"
#else
	#define CPU_TYPE "32 BIT"
#endif // WIN64
#ifdef _DEBUG
	#define BLD_TYPE ", RELEASE"
#else
	#define BLD_TYPE ", DEBUG"
#endif 
		log_trace(CPU_TYPE BLD_TYPE	" BUILD" );
#undef CPU_TYPE
#undef BLD_TYPE

		driver(buffer_size / 1); 	
		driver(buffer_size / 2);   	
		driver(buffer_size / 4); 	
		driver(buffer_size / 8); 	
		driver(buffer_size / 16); 	
		driver(buffer_size / 32); 	

		show_results();

		});
} // namespace measure_buffers_performance 
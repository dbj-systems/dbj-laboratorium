#pragma once

/*
http://www.strudel.org.uk/itoa/
*/

namespace dbj::sample {

	/**
		 * C++ version 0.4 char* style "itoa":
		 * Written by Lukás Chmela
		 * Released under GPLv3.

		 dbj changed from this :
			char* itoa(int value, char* result, int base) {
		 to this :
			char* itoa(int value, int base) {

		 */
	inline char* itoa_lukas(int value, int base) {

		static char result[36]{ 0 };	::memset(result, 0, 36);
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

	inline std::string itoa_original(int value, int base) {

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

	char_buf itoa_unique_ptr(int value, int base) 
	{
		return ::dbj::buf::smart( std::string_view( itoa_lukas(value, base) ) );
#if 0
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
#endif
	}

	std::array<char, 35> itoa_array(int value, int base)
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

	inline const auto iterations_count = 0xFFFFF;
	inline const auto integer_to_transform = 0xFFFF;

	DBJ_TEST_UNIT(measure_strudel_itoa)
	{
		auto report = [&](auto title, auto fun_)
		{
			auto runner = [&]() {
				static int value = integer_to_transform; 
				static int base = 10;
				for (int k{}; k < iterations_count; k++)
					(void)fun_(value, base);
			};

			using dbj::fmt::print;
			print("\nalgorithm: %s,\n\tresult: %29s",
				title,
				dbj::kalends::miliseconds_measure(runner)
			);
		};
		using dbj::fmt::print;

		print("\n\nIterations: %d\nHeap based\n", iterations_count );
		report("strudel_vector", itoa_vector);
		report("strudel_original", itoa_original);
		report("strudel_dbj", itoa_unique_ptr);
		print("\n\nStack based\n");
		report("strudel_array", itoa_array);
		report("strudel_c", itoa_lukas);
		
		::system("@echo.");
		::system("@echo.");
		::system("@pause");

	}

} // dbj::samples
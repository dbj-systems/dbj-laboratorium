#pragma once

//#include "../core/dbj_traits.h"
//#include <stdint.h>
//#include <random>

namespace dbj::num {

	/*
	perhaps naughty but surely simple and working
	*/
	namespace float_to_integer
	{
		inline auto nearest = [](float value_) constexpr -> int {
			return (int)((int)(value_ + 0.5));
		};
		inline auto drop_fractional_part = [](float value_) constexpr -> int {
			return (int)((int)value_);
		};
		inline auto next_highest = [](float value_) constexpr -> int  {
			return (int)((int)(value_ + 0.9));
		};
		inline auto  integer = [](float value_) constexpr -> int {
			return (int)value_;
		};
	};

	/*
	This is supposed to be the "fastest" itoa() implementation
	Taken from http://fmtlib.net/latest/index.html
	Author: "vitaut"
	*/

	constexpr inline uint16_t const str100p[100] = {
	0x3030, 0x3130, 0x3230, 0x3330, 0x3430, 0x3530, 0x3630, 0x3730, 0x3830, 0x3930,
	0x3031, 0x3131, 0x3231, 0x3331, 0x3431, 0x3531, 0x3631, 0x3731, 0x3831, 0x3931,
	0x3032, 0x3132, 0x3232, 0x3332, 0x3432, 0x3532, 0x3632, 0x3732, 0x3832, 0x3932,
	0x3033, 0x3133, 0x3233, 0x3333, 0x3433, 0x3533, 0x3633, 0x3733, 0x3833, 0x3933,
	0x3034, 0x3134, 0x3234, 0x3334, 0x3434, 0x3534, 0x3634, 0x3734, 0x3834, 0x3934,
	0x3035, 0x3135, 0x3235, 0x3335, 0x3435, 0x3535, 0x3635, 0x3735, 0x3835, 0x3935,
	0x3036, 0x3136, 0x3236, 0x3336, 0x3436, 0x3536, 0x3636, 0x3736, 0x3836, 0x3936,
	0x3037, 0x3137, 0x3237, 0x3337, 0x3437, 0x3537, 0x3637, 0x3737, 0x3837, 0x3937,
	0x3038, 0x3138, 0x3238, 0x3338, 0x3438, 0x3538, 0x3638, 0x3738, 0x3838, 0x3938,
	0x3039, 0x3139, 0x3239, 0x3339, 0x3439, 0x3539, 0x3639, 0x3739, 0x3839, 0x3939, };

	/*
	you send the buf, so make it 'big enough'
	*/
	inline char * itoa_vitaut_1(char *buf, uint32_t val) noexcept
	{
		char *p = &buf[10];

		*p = '\0';

		while (val >= 100)
		{
			uint32_t const old = val;

			p -= 2;
			val /= 100;
			::memcpy(p, &str100p[old - (val * 100)], sizeof(uint16_t));
		}

		p -= 2;
		::memcpy(p, &str100p[val], sizeof(uint16_t));

		return &p[val < 10];
	}

	inline std::array<char, 64> itos(long l_) noexcept
	{
		std::array<char, 64> str{ {0} };

		[[maybe_unused]] auto[p, ec]
			= std::to_chars(str.data(), str.data() + str.size(), l_);
		_unused(p);
		_ASSERTE(ec != std::errc::value_too_large);
		return str;
	}

	/*
	*/
	template<typename INT_TYPE_>
	INT_TYPE_ random_from_to(INT_TYPE_ min, INT_TYPE_ max)
	{
		static_assert(::dbj::is_any_same_as_first_v<INT_TYPE_,
			short, int, long, long long, unsigned short, unsigned int,
			unsigned long, unsigned long long>,
			"\n\nInvalid template argument for " __FUNCSIG__ ": requires one of \n"
			"short, int, long, long long, unsigned short, unsigned int, unsigned long, or unsigned long long\n\n");

		std::random_device seed;
		std::mt19937 rng(seed());
		std::uniform_int_distribution<INT_TYPE_> gen(min, max); // uniform, unbiased
		INT_TYPE_ rand_val = gen(rng);
		return rand_val;
	};

	// inferior to above, almost "C like" ... so why is it in here then?
	inline auto random = [](int max_val, int min_val = 1) -> int {
		static auto initor = []() {
			std::srand((unsigned)std::time(nullptr)); return 0;
		}();
		return ::abs(int(min_val + std::rand() / ((RAND_MAX + 1u) / max_val)));
	};

	inline constexpr std::int32_t INT_MAX_OVER_10 = INT_MAX / 10;
	inline constexpr std::int32_t INT_MIN_OVER_10 = INT_MIN / 10;

	// reverse the 32 bit integer
	// not my code, just optimized this https://leetcode.com/articles/reverse-integer/
	extern "C" constexpr std::int32_t reverse(std::int32_t x) noexcept
	{

		std::int32_t rev = 0;
		while (x != 0) {
			std::int32_t pop = x % 10;
			x /= 10;
			/*
			Explanation:
			The maximun of plus integer is 2^32 - 1,which mod 10 is 7,
			The minimum of minus integer is -2^32,which mod 10 is 8.
			*/
			if (rev > INT_MAX_OVER_10 || (rev == INT_MAX_OVER_10 && pop > 7)) return 0;
			if (rev < INT_MIN_OVER_10 || (rev == INT_MIN_OVER_10 && pop < -8)) return 0;
			rev = rev * 10 + pop;
		}
		return rev;
	}

	/*
	the "clever" factorial is just a lookup,
	since for 32bit int overflow happens for 13!

	fact values source: http://www.tsm-resources.com/alists/fact.html

	reminder:
	#define INT32_MAX        2147483647i32
	#define INT64_MAX        9223372036854775807i64
	*/

	constexpr std::int32_t factorials32[]{ 1, 1, 2, 6, 24, 120, 720,
			5040, 40320, 362880, 3628800, 39916800, 479001600 };

	extern "C" constexpr
		std::int32_t const &
		fact32(std::int32_t const & i) {
		if (i < 0U || i> 12U) {
			perror("\n\n" __FUNCSIG__ " -- argument out of range\n\n");
			return std::int32_t(std::errc::result_out_of_range);
		}
		return factorials32[i];
	}

	constexpr  std::int64_t factorials64[]
	{ 1,
			1,
			2,
			6,
			24,
			120,
			720,
			5040, 40320, 362880, 3628800, 39916800, 479001600,
			6227020800, /*13!*/
			87178291200,
			1307674368000,
			20922789888000,
			355687428096000,
			6402373705728000,
			121645100408832000,
			2432902008176640000 /*20!*/
	};
	extern "C" constexpr
		std::int64_t const &
		fact64(std::int64_t const & i) {

		if (i < 0U || i> 20U) {
			perror("\n\n" __FUNCSIG__ " -- argument out of range\n\n");
			// POSIX
			return std::int64_t(std::errc::result_out_of_range);
		}
		return factorials64[i];
	}

	using fibo_type = int64_t;

		/*
		this is the best runtime fibonacci
		non recursive
		compiler is free to optimize this in the best possible way
		on i5 CPU / 8GB ram x64 WIN10 this was the fastest for N=40
		*/
		extern "C" constexpr fibo_type fibo (fibo_type n)
		{
			fibo_type c = 0, a = 1, b = 1;
			for (fibo_type i = 3; i <= n; i++) {
				c = a + b;
				a = b;
				b = c;
			}
			return b;
		}

} // dbj::num
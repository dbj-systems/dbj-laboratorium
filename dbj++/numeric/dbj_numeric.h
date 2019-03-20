#pragma once
#include <stdint.h>

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

    while(val >= 100)
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
	DBJ_NOUSE(p);
	_ASSERTE(ec != std::errc::value_too_large);
	return str ;
}

} // dbj::num
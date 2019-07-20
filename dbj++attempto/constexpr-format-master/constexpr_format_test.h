#pragma once
#include "../pch.h"
#include "constexpr_format.h"

DBJ_TEST_UNIT(compile_time_formatter)
{
	namespace cf = constexpr_format;
	using namespace constexpr_format::string_udl;
	constexpr auto s = "Hello %%%s%%, this is number %d and %d"_sv; 

	cf::util::string_view sv{"Hola Lola!"};
}

namespace therocode {
	template <typename StringHolder>
	constexpr auto parseNumber(StringHolder holder)
	{
		constexpr std::string_view text = holder();

		//we only support strings of length 3
		static_assert(text.size() == 3, "invalid length of input");

		constexpr char typeChar = text[0];
		constexpr char numberChar = text[2];

		//type char must be i or f
		static_assert(typeChar == 'i' || typeChar == 'f', "must start with 'i' or 'f'");
		//must be a colon as second character
		static_assert(text[1] == ':', "lacks proper ':' delimeter");

		//number char must have 0-9 as their ascii value
		static_assert(numberChar >= '0' && numberChar <= '9', "number part is not a valid number");

		int resultingNumber = numberChar - '0';
		if constexpr (typeChar == 'i')
			return resultingNumber;
		else
			return static_cast<float>(resultingNumber);
	}

#define PARSE_NUMBER(text) \
parseNumber([](){return text;})

	// https://blog.therocode.net/2018/09/compile-time-string-parsing
	DBJ_TEST_UNIT(therocodes_blog)
	{
		auto intResult = PARSE_NUMBER("i:2");
		auto floatResult = PARSE_NUMBER("f:5");

		static_assert(std::is_same_v<decltype(intResult), int>);
		static_assert(std::is_same_v<decltype(floatResult), float>);
	}

#undef PARSE_NUMBER

} // ns therocode
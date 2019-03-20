#pragma once
#include <typeinfo>

DBJ_TEST_SPACE_OPEN(dbj_crt__tests)

DBJ_TEST_UNIT(testingdbjstris_std_string_v)
{
	static_assert( dbj::is_std_string_v<std::string> );
	static_assert(dbj::is_std_string_v<std::wstring>);
	static_assert(dbj::is_std_string_v<std::u16string>);
	static_assert(dbj::is_std_string_v<std::u32string>);
	static_assert(! dbj::is_std_string_v<char *>);
	static_assert(! dbj::is_std_string_v<const char *>);
	static_assert(! dbj::is_std_string_v<char(&)[]>);
	static_assert(! dbj::is_std_string_v<char(*)[]>);
	static_assert(! dbj::is_std_string_v<char[]>);
}

DBJ_TEST_UNIT(testingdbjstris_std_char_v)
{
	static_assert(dbj::is_std_char_v<char>);
	static_assert(dbj::is_std_char_v<wchar_t>);
	static_assert(dbj::is_std_char_v<char16_t>);
	static_assert(dbj::is_std_char_v<char32_t>);
	static_assert(dbj::is_std_char_v<char *>);
	static_assert(dbj::is_std_char_v<const char *>);
	static_assert(dbj::is_std_char_v<char & >);
	static_assert(dbj::is_std_char_v<char &&>);
	static_assert(dbj::is_std_char_v<const wchar_t >);
}

DBJ_TEST_SPACE_CLOSE
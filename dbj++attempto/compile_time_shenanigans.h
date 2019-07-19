#pragma once
#include "pch.h"
// https://stackoverflow.com/a/51776569
//#include <cassert>
//#include <cstddef>
//#include <iostream>

namespace dbj::samples {
	template<class T>
	constexpr const char* getName();

	template<>
	constexpr const char* getName<int>() {
		return "int";
	}

	template<std::size_t N>
	constexpr const char* getNumericString();

	template<>
	constexpr const char* getNumericString<16>() {
		return "16";
	}

	constexpr std::size_t get_strLen(const char* str) {
		std::size_t ret = 0;
		while (str[ret] != '\0') ret++;
		return ret;
	}

	static_assert(get_strLen("") == 0, "");
	static_assert(get_strLen("ab") == 2, "");
	static_assert(get_strLen("4\0\0aaa") == 1, "");

	class str_literal_type final 
	{
		const char* str;

	public:

		constexpr str_literal_type(const char * cp_ ) : str(cp_) {}

		constexpr auto data() const { return str; }

		constexpr auto begin() const { return str; }

		constexpr auto end() const {
			auto it = str;
			while (*it != '\0') ++it;
			return it;
		}

		constexpr std::size_t length() const noexcept {
			std::size_t ret = 0;
			while (str[ret] != '\0') ret++;
			return ret;
		}
	};

	//template<class T, std::size_t size>
	//class array {
	//private:
	//	T data_[size]{};
	//public:
	//	constexpr T& operator[](std::size_t i) { return data_[i]; }
	//	constexpr const T& operator[](std::size_t i) const { return data_[i]; }
	//	constexpr const T* data() const { return data_; }
	//};

	//template<std::size_t buffer_size, class... Args>
	//constexpr array<char, buffer_size> cat( const Args... args) {
	//	array<char, buffer_size> ret{};

	//	std::size_t i = 0;
	//	constexpr auto argslist = { str_literal_type{ args }...};
	//	for (auto arg : argslist ) {
	//		for (char c : arg) ret[i++] = c;
	//	}

	//	return ret;
	//}

	//template<class T, std::size_t N>
	//struct StaticDataForConstexprFunction {
	//	static constexpr const char* name = getName<T>();
	//	static constexpr const char* length = getNumericString<N>();
	//	static constexpr std::size_t size = get_strLen(name) + get_strLen(length) + 10;

	//	using Buffer = array<char, size>;
	//	static constexpr Buffer buffer = cat<size>(name, "[", length, "]\0");
	//};

	//template<class T, std::size_t N>
	//constexpr typename StaticDataForConstexprFunction<T, N>::Buffer StaticDataForConstexprFunction<T, N>::buffer;

	//template<class T, std::size_t N>
	//constexpr const char* getName(T(&)[N]) {
	//	return StaticDataForConstexprFunction<T, N>::buffer.data();
	//}

	using namespace std::literals;

	DBJ_TEST_UNIT(compile_time_shenanigans)
	{
		constexpr auto sview = "Hola Lola!"sv;
		constexpr auto len = get_strLen( sview.data() ) ;
		constexpr str_literal_type str{ "Wot?!" };
		constexpr auto length_ = str.length();

		constexpr std::array<char, len > buffer{ { 0 } };
		
		dbj::fmt::print("\n%d\n", len);
	}

} // namespace dbj::samples 
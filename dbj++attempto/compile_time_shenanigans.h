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

		constexpr str_literal_type(const char* cp_) : str(cp_) {}

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

	namespace buffer {
		template<size_t N >
		constexpr auto buffer(const char(&sl_)[N])
		{
			std::array<char, N + 1 > buffer_{ { 0 } };

			size_t k = 0;
			for (auto chr : sl_) {
				buffer_[k++] = chr;
			}

			return buffer_;
		}

	} // ns buffer 

	DBJ_TEST_UNIT(compile_time_shenanigans)
	{
		constexpr auto				sview = "Hola Lola!"sv;
		constexpr auto				len = get_strLen(sview.data());
		constexpr str_literal_type	str{ "Wot?!" };
		constexpr auto				length_ = str.length();

		constexpr auto buffer_1 = buffer::buffer("Hola Lola!");
		constexpr std::array<char, buffer_1.size() > buffer_2 = buffer_1;


		dbj::fmt::print("\n%d\n", len);
	}

} // namespace dbj::samples 

// This file is a "Hello, world!" in C++ language by GCC for wandbox.
#include <iostream>
#include <string>
#include <array>
#include <functional>

#ifndef DBJ_TYPENAME
#define DBJ_TYPENAME(T) typeid(T).name() 
#endif // !DBJ_TYPENAME

namespace dbj::util {
	/*
	// https://stackoverflow.com/a/18682805/10870835
	template<typename T>
	class has_type
	{
		typedef struct { char c[1]; } yes;
		typedef struct { char c[2]; } no;

		template<typename U> static constexpr yes test(typename U::type);
		template<typename U> static constexpr no  test(...);

	public:
		static constexpr bool result = sizeof(test<T>(nullptr)) == sizeof(yes);
	};
	*/


} // dbj::util 

// https://wandbox.org/permlink/HDbvC6PP33BMVTbA
#if 0
namespace faux_fp {
	using namespace std;
	using namespace std::literals;
	// #define DBJ_TX(x) do { std::cout << std::boolalpha << "\n\nExpression: '" << #x << "'\n\tResult: " << (x) << "\n\tIt's type: " << typeid(x).name()<< "\n\n"; } while (0)

	template<typename T, typename ... A>
	using RequiredSignature = bool(T&, A ... a);

	template<typename T, typename ... A>
	using RequiredFP = bool(*)(T&, A ... a);

	bool ok_fun(int& a) { cout << "\n" << "bool ok_fun(int& " << a << ")";  return true; }
	void wrong_fun() { cout << "\n" << "void wrong_fun( )"; }



	namespace inner {
		typedef char yes[1];
		typedef char no[2];

		template<typename T>
		yes& is_same_helper(T, T);  //no need to define it now!

		template<typename T, typename U>
		no& is_same_helper(T, U); //no definition needed!

		template<typename T, typename U>
		constexpr inline bool fp_matches_fp( T x, U y)
		{
			// return (sizeof(is_same_helper(x, y)) == sizeof(yes));
			return (sizeof(is_same_helper(x, y)) == sizeof(yes));
		} 

	}

	DBJ_TEST_UNIT(signature_matches_function_pointer)
	{
		//int forty_two = 42;

		using required_sig = RequiredSignature<int>;

		if constexpr (dbj::util::signature_fp_match< required_sig >(ok_fun)) {
			DBJ_FPRINTF(stdout, "\n\n%sdoes  confirm to the required signature:\n%s ",
				DBJ_TYPENAME(decltype(ok_fun)),
				DBJ_TYPENAME(required_sig));
		}

		if constexpr (!dbj::util::signature_fp_match< required_sig >(wrong_fun)) {
			DBJ_FPRINTF(stdout, "\n\n%sdoes not confirm to the required signature:\n%s ",
				DBJ_TYPENAME(decltype(wrong_fun)),
				DBJ_TYPENAME(required_sig));
		}
	}
} // ns
#endif
#ifndef _DBJ_META_CONVERTER_INC_
#define _DBJ_META_CONVERTER_INC_
/*
(c) 2019 by dbj@dbj.org
LICENCE -- CC BY SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/

Q: what about string views?
A: this is not "in place" transformation.
we create new strings of required
type and populate them with transformed content

NOTE: be forewarned in C++20 you will not be able to output char8_t *, char16_t * and char32_t *
as easily as in C++17. in particular std::cout will be barred

The full jamboree is here:
https://stackoverflow.com/questions/58878651/what-is-the-printf-formating-character-for-char8-t

printf() might be the way out, and <uchar.h> otherwise.

thus the macto _TEST bellow does not get in that mess ...
*/

#include <type_traits>
#include <string>

namespace dbj {

	namespace typetraits {
		using namespace std;

		template <typename T> struct remove_all_ptr { typedef T type; };

		template <typename T> struct remove_all_ptr<T*> {
			using type = typename remove_all_ptr<::std::remove_cv_t<T>>::type;
		};

		// reduce T***** to T, for any level of pointers to pointers
		template <typename T>
		using remove_all_ptr_t = typename remove_all_ptr<T>::type;

#if __cplusplus <= 201703L
		// NOT C++20 code
		// ::std::remove_cvref is to be in the C++20
		template< class T >
		struct remove_cvref {
			typedef ::std::remove_cv_t<::std::remove_reference_t<T>> type;
		};

		template< class T >
		using remove_cvref_t = typename remove_cvref<T>::type;
#endif

		// reduce any compound type , to its base type
		template <class T>
		using to_base_t =
			remove_all_ptr_t< ::std::remove_all_extents_t< remove_cvref_t < T > > >;

		// dbj.org 2018-07-03
		// NOTE: pointers are not char's
		// char *. wchar_t * .. are thus not chars	
		// take care of chars and their signed and unsigned forms
		// where 'char' means one of the four ::std char types
		// NOTE: u8string we will skip for now
		// as it stands at 2019Q4 "char8_t is a mess" in C++20

		template<class _Ty>	struct is_char : ::std::false_type {	};
		template<> struct is_char<char> : ::std::true_type {	};
		template<> struct is_char<signed char> : ::std::true_type {	};
		template<> struct is_char<unsigned char> : ::std::true_type {	};

		template<typename T> inline constexpr bool  is_char_v = is_char<T>::value;

		template<class _Ty>	struct is_wchar : ::std::false_type {	};
		template<> struct is_wchar<wchar_t> : ::std::true_type {	};

		template<typename T> inline constexpr bool  is_wchar_v = is_wchar<T>::value;

		template<class _Ty>	struct is_char16 : ::std::false_type {	};
		template<> struct is_char16<char16_t> : ::std::true_type {	};

		template<typename T> inline constexpr bool  is_char16_v = is_char16<T>::value;

		template<class _Ty>	struct is_char32 : ::std::false_type {	};
		template<> struct is_char32<char32_t> : ::std::true_type {	};

		template<typename T> inline constexpr bool  is_char32_v = is_char32<T>::value;

#if __cplusplus > 201703L
		// C++20 code
		template<class _Ty>	struct is_char8 : ::std::false_type {	};
		template<> struct is_char8<char8_t> : ::std::true_type {	};

		template<typename T> inline constexpr bool  is_char8_v = is_char8<T>::value;
#endif

		/************************************************************************************/
		// and one for all
		template<typename T, typename base_t = to_base_t<T> >
		struct is_std_char :
			::std::integral_constant
			<
			bool,
			is_char_v< base_t > || is_wchar_v<base_t> ||
			is_char16_v<base_t> || is_char32_v<base_t>
#if __cplusplus > 201703L
			|| is_char8_v<base_t>
#endif
			>
		{};

		template<typename T >
		inline constexpr bool  is_std_char_v = is_std_char<T>::value;

		// is T, a standard string
		template< class T >
		struct is_std_string : integral_constant < bool,
			is_same_v<T, string    > ||
			is_same_v<T, wstring   > ||
			is_same_v<T, u16string > ||
			is_same_v<T, u32string >
#if __cplusplus > 201703L
			|| is_same_v<T, u8string >
#endif
			>
		{};

		template<typename T>
		inline constexpr bool  is_std_string_v = is_std_string<T>::value;

		///<summary>
		/// as per official definition circa 2019Q4
		/// range is anything that has 
		/// begin() and end() methods
		/// But. We add yet another pre condition
		/// T::value_type must be present
		/// and then we give it a long and descriptive name
		/// so that users do not use it wrongly
		///</summary>
		template <typename T, typename = void>
		struct is_range_with_value_type final : ::std::false_type {};

		template <typename T>
		struct is_range_with_value_type <T
			, ::std::void_t
			<
			decltype(::std::declval<T>().begin()),
			decltype(::std::declval<T>().end()),
			typename T::value_type
			>
		> final : ::std::true_type{};

		template<typename T>
		constexpr inline bool is_range_with_value_type_v = is_range_with_value_type<T>::value;

	} // typetraits

	namespace inner {
		/// <summary>
		/// Try to convert any range made of standard char types
		/// return type is one of std strings
		/// range is anything that has begin() and end(), and 
		/// value_type typedef as per std containers model
		/// </summary>
		template < typename return_type >
		struct meta_converter final
		{
			static_assert(typetraits::is_std_string_v<return_type>, "return type must be standard string type");

			template<typename T>
			return_type operator () (T arg)
			{
				// using namespace typetraits;

				using actual_type = typetraits::remove_cvref_t< T >;

				if constexpr (std::is_same_v<actual_type, return_type >) {
					return arg; // copy to output
				}
				else {

					static_assert(typetraits::is_range_with_value_type_v<actual_type>,
						"can transform only ranges also having a T::value_type accessible");

					static_assert (
						// arg must have nested value_type 
						typetraits::is_std_char_v< typename actual_type::value_type >,
						"can not transform ranges **not** made out of standard char types"
						);
#ifndef _MSC_VER
					return { arg.begin(), arg.end() };
#else
					// currently a lot of warnings C4244 
					// for MSVC, as of 2019-11-27
#pragma warning( push )
#pragma warning ( disable: 4244 )
					return return_type(arg.begin(), arg.end());
#pragma warning( pop )
#endif
				}
			}

			// dealing with native string literals
			template<typename T, size_t N>
			return_type operator () (const T(&arg)[N])
			{
				using actual_type
					= std::remove_cv_t< std::remove_pointer_t<T> >;

				static_assert (
					typetraits::is_std_char_v< actual_type >,
					"can transform only literals made out of standard char types"
					);

				// make string and send it to 
				// range transformation method
				return this->operator()(
					std::basic_string<actual_type>{ arg }
				);
			}

			// native poiinters are not allowed
			// that would rely on zero terminated strings
			// and that is a safety risk

		}; // meta_converter

		   // explicit instantiations
		template struct meta_converter<std::string   >;
		template struct meta_converter<std::wstring  >;
		template struct meta_converter<std::u16string>;
		template struct meta_converter<std::u32string>;
#if __cplusplus > 201703L
		// C++20
		template struct meta_converter<std::u8string>;
#endif

	} // inner

	// implicit instantiations
	inline dbj::inner::meta_converter<std::string   > range_to_string{};
	inline dbj::inner::meta_converter<std::wstring  > range_to_wstring{};
	inline dbj::inner::meta_converter<std::u16string> range_to_u16string{};
	inline dbj::inner::meta_converter<std::u32string> range_to_u32string{};
#if __cplusplus > 201703L
	// C++20
	inline dbj::inner::meta_converter<std::u8string> range_to_u8string{};
#endif
} // dbj 

// TESTING

#ifdef _DBJ_META_CONVERTER_TESTING

#include <deque>
#include <forward_list>
#include <array>
#include <vector>

// just show the type of the result ...
#define TEST_(x) do { \
auto const & rez = (x); \
std::cout << std::endl << _CRT_STRINGIZE(x) << "\n result type: " << typeid(rez).name() << std::endl; \
} while(false)

namespace meta_conversion_testing {

	using namespace std;

	/// <summary>
	///  the string meta converter testing
	/// </summary>
	template<typename converter_type>
	inline void test_conversion(converter_type&& the_converter)
	{
		using namespace std;
		using namespace std::string_literals;
		// standard string literals
		TEST_(the_converter("the\0\0standard string literal"s));
		TEST_(the_converter(L"the\0\0standard string literal"s));
		TEST_(the_converter(u"the\0\0standard string literal"s));
		TEST_(the_converter(U"the\0\0standard string literal"s));
#if __cplusplus > 201703L
		TEST_(the_converter(u8"the\0\0standard string literal"s));
#endif	

		// native string literals
		TEST_(the_converter("Abra Ca Dabra Alhambra"));
		TEST_(the_converter(L"Abra Ca Dabra Alhambra"));
		TEST_(the_converter(u"Abra Ca Dabra Alhambra"));
		TEST_(the_converter(U"Abra Ca Dabra Alhambra"));
#if __cplusplus > 201703L
		TEST_(the_converter(u8"Abra Ca Dabra Alhambra"));
#endif	

		// the five C++20 standard string types
		TEST_(the_converter(std::string{ "Abra Ca Dabra Alhambra" }));
		TEST_(the_converter(std::wstring{ L"Abra Ca Dabra Alhambra" }));
		TEST_(the_converter(std::u16string{ u"Abra Ca Dabra Alhambra" }));
		TEST_(the_converter(std::u32string{ U"Abra Ca Dabra Alhambra" }));
#if __cplusplus > 201703L
		TEST_(the_converter(std::u8string{ u8"Abra Ca Dabra Alhambra" }));
#endif	

		// now let's try the renge-like containers
		// in case you have missed it we can transform any range of chars
		// into any required std string type

		TEST_(the_converter(std::array{ 'H', 'e', 'l', 'l', 'o', '!' }));
		TEST_(the_converter(std::array{ L'H', L'e', L'l', L'l', L'o', L'!' }));
		TEST_(the_converter(std::array{ u'H', u'e', u'l', u'l', u'o', u'!' }));
		TEST_(the_converter(std::array{ U'H', U'e', U'l', U'l', U'o', U'!' }));
#if __cplusplus > 201703L
		TEST_(the_converter(std::array{ u8'H', u8'e', u8'l', u8'l', u8'o', u8'!' }));
#endif	

		TEST_(the_converter(std::vector{ 'H', 'e', 'l', 'l', 'o', '!' }));
		TEST_(the_converter(std::vector{ L'H', L'e', L'l', L'l', L'o', L'!' }));
		TEST_(the_converter(std::vector{ u'H', u'e', u'l', u'l', u'o', u'!' }));
		TEST_(the_converter(std::vector{ U'H', U'e', U'l', U'l', U'o', U'!' }));
#if __cplusplus > 201703L
		TEST_(the_converter(std::vector{ u8'H', u8'e', u8'l', u8'l', u8'o', u8'!' }));
#endif	

		TEST_(the_converter(std::deque{ 'H', 'e', 'l', 'l', 'o', '!' }));
		TEST_(the_converter(std::deque{ L'H', L'e', L'l', L'l', L'o', L'!' }));
		TEST_(the_converter(std::deque{ u'H', u'e', u'l', u'l', u'o', u'!' }));
		TEST_(the_converter(std::deque{ U'H', U'e', U'l', U'l', U'o', U'!' }));
#if __cplusplus > 201703L
		TEST_(the_converter(std::deque{ u8'H', u8'e', u8'l', u8'l', u8'o', u8'!' }));
#endif	

		TEST_(the_converter(std::forward_list{ 'H', 'e', 'l', 'l', 'o', '!' }));
		TEST_(the_converter(std::forward_list{ L'H', L'e', L'l', L'l', L'o', L'!' }));
		TEST_(the_converter(std::forward_list{ u'H', u'e', u'l', u'l', u'o', u'!' }));
		TEST_(the_converter(std::forward_list{ U'H', U'e', U'l', U'l', U'o', U'!' }));
#if __cplusplus > 201703L
		TEST_(the_converter(std::forward_list{ u8'H', u8'e', u8'l', u8'l', u8'o', u8'!' }));
#endif	

		// no can do -- only ranges and arrays aka string literals
		// char const * native_and_nasty = "Buahaha!";
		// TEST_(the_converter(native_and_nasty));


		// also we do not serve stunt men any more
		//TEST_(the_converter(static_cast< char(&)[]>("Abra Ca Dabra")));
		//TEST_(the_converter(static_cast< wchar_t(&)[]>(L"Abra Ca Dabra")));
		//TEST_(the_converter(static_cast< char16_t(&)[]>(u"Abra Ca Dabra")));
		//TEST_(the_converter(static_cast< char32_t(&)[]>(U"Abra Ca Dabra")));

		// then string views, and so on ...
		// now lets test non char containers
		// rightly so, none of this compiles
		// TEST_(the_converter(std::array<int, 6>{  1,2,3,4,5,6 }));
		// TEST_(the_converter(std::vector<int>{  1, 2, 3, 4, 5, 6 }));
		// TEST_(the_converter(std::basic_string<int>{  1, 2, 3, 4, 5, 6 }));
	}

	inline void metaconvertertest() {

		test_conversion(dbj::range_to_string);
		test_conversion(dbj::range_to_wstring);
		test_conversion(dbj::range_to_u16string);
		test_conversion(dbj::range_to_u32string);
#if __cplusplus > 201703L
		test_conversion(dbj::range_to_u8string);
#endif	
	}
} // namespace meta_conversion_testing

#undef TEST_
#endif // _DBJ_META_CONVERTER_TESTING

#endif // !_DBJ_META_CONVERTER_INC_

#pragma once

namespace dbj::util {
#pragma region dealing with optional<T>
	/*
	Why std::optional?

	Examples:
	1.
	It is extremely preferable to having something like a std::optional<std::string>
	for each string member! std::optional gives you data locality, which is great
	for performance.
	2.
	std::optional<int> try_parse_int(std::string )
	above is **much** simpler/safer/faster vs
	int * try_parse_int(std::string )
		or
	bool try_parse_int(std::string , int & )
	3.
	Avoiding '-1' as 'special value'
	std::optional<int> find_in_string(std::string , std::string );

	 it appears bellow is all we need to
	 handle the optional<T> in a comfortable safer manner
	 optional<T> actually never looses it's 'T'
	 bellow is with added functionality of user
	 defined dflt optional val
	*/

	template<typename T>
	inline T optival (std::optional<T> opt, T dflt_ = T{})
		noexcept // daring
	{
		return opt.value_or(dflt_);
	}

// same as above but adding "meta default" 
// and delivering optional<T> instance
// returning the two above in an std::pair
	template<typename T>
	inline auto optional_pair(T defval = {})
	{
// here we enforce type requirements for T
// we specificaly bar the pointers handling
// smart pointers are made for that
		static_assert(
			!is_pointer<T>::value,
			__FILE__ " optional_handle -- T can not be a pointer"
			);
// somewhat controversial requirement ;)
// https://foonathan.net/blog/2018/07/12/optional-reference.html
		static_assert(
			!is_reference_v <T>,
			__FILE__ " optional_handle -- T can not be a reference"
			);
// here add static_assert's to narrow the choice of the type allowed
		static_assert(std::is_constructible_v<T>,
			"\n\n" __FUNCSIG__ "\nT must be constructible\n");
		static_assert(std::is_copy_constructible_v<T>,
			"\n\n" __FUNCSIG__ "\nT must be copy constructible\n");
		static_assert(std::is_move_constructible_v < T >,
			"\n\n" __FUNCSIG__ "\nT must be move constructible\n");

		return std::make_pair(
// the first
// pay attention: we do not need "functor" here
			[=]
		(std::optional<T> opt_, std::optional<T>  dflt_ = std::nullopt)
			noexcept -> T
		{
			T meta_def{ defval };
			return opt_.value_or(dflt_.value_or(meta_def));
		},
			// the second
			std::optional<T>{}
		);
	} // optional_pair
#pragma endregion

#pragma region dealing with std::any
/*
any and optional and pair working together
this makes the any much more usefull but
defeats the purpose of having it at all
*/
	template<typename T>
	auto make_any_pair(T const & v_ = 0)
	{
		using value_type = std::decay_t< T >;

		static_assert(!std::is_reference_v< value_type >,
			"\n\n[dbj make_any_pair] Can not hold a reference type\n");

		static_assert(!std::is_array_v< value_type >,
			"\n\n[dbj make_any_pair] Can not hold an array type\n");

		static_assert(std::is_constructible_v < value_type >,
			"\n\n[dbj make_any_pair] T must be constructible\n");

		static_assert(std::is_move_constructible_v < value_type >,
			"\n\n[dbj make_any_pair] T must be move constructible\n");

		(void)noexcept(v_); // pacify the warning about unused

		using optitype = std::optional< T >;

		return
			std::make_pair(
				[=](typename ::std::any any_) -> optitype
		{
			if (any_.has_value()) {
				if (auto ptr = std::any_cast<T>(&any_); ptr) {
					// ptr is not nullptr here 
					return optitype{ *ptr };
				}
			}
			return std::nullopt;
		},
				std::any{ v_ }
		);
	} // make_any_pair

#pragma endregion
} // dbj

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
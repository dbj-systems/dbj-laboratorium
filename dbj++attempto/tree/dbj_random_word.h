#pragma once
#include <array>
#include <algorithm>
#include <string_view>
#include <ctime>

/*
produce radnom words but just a bit more normal
of not, or just numbers, or just vowels ...
whatever you like
*/
namespace dbj_tree_research
{

using namespace std;
using namespace std::string_view_literals;

template <typename CT>
struct meta_data final
{
};

template <>
struct meta_data<char> final
{
	constexpr static auto digits{"0123456789"sv};
	constexpr static auto alpha{"bcdfghjklmnpqrstvwxyz"sv}; // no vowels
	constexpr static auto vowels{"aeiou"sv};

	constexpr static auto digits_len = digits.size();
	constexpr static auto alpha_len = alpha.size();
	constexpr static auto vowels_len = vowels.size();
};

template <>
struct meta_data<wchar_t> final
{
	constexpr static auto digits{L"0123456789"sv};
	constexpr static auto alpha{L"bcdfghjklmnpqrstvwxyz"sv}; // no vowels
	constexpr static auto vowels{L"aeiou"sv};

	constexpr static auto digits_len = digits.size();
	constexpr static auto alpha_len = alpha.size();
	constexpr static auto vowels_len = vowels.size();
};

// good enough for the purpose
inline auto random = [](int max_val, int min_val = 1)
	-> std::size_t {
	static auto initor = []() {
		std::srand((unsigned)std::time(nullptr));
		return 0;
	}();
	return ::abs(int(min_val + std::rand() / ((RAND_MAX + 1u) / max_val)));
};

template <typename CT>
inline auto digits_c = []() constexpr -> CT
{
	return meta_data<CT>::digits[random(meta_data<CT>::digits_len) - 1];
};

template <typename CT>
inline auto alpha_c = []() constexpr -> char
{
	return meta_data<CT>::alpha[random(meta_data<CT>::alpha_len) - 1];
};

template <typename CT>
inline auto vowels_c = []() constexpr -> char
{
	return meta_data<CT>::vowels[random(meta_data<CT>::vowels_len) - 1];
};

enum class kind
{
	digits,
	alpha,
	vowels,
	alpha_vowels,
	alpha_vowels_digits
};

/*
	make random words of the "kind" as enum above allows
	default is leters and vowels
	to make things simple and fast
	use std::array
	*/
template <typename T, size_t L>
constexpr inline array<T, L> const random_word(
	array<T, L> carr, kind which_ = kind::alpha_vowels)
{
	std::size_t N = L - 1;

	switch (which_)
	{
	case kind::digits:
		for (auto k = 0U; k < N; k++)
			carr[k] = digits_c<T>();
		break;
	case kind::alpha:
		for (auto k = 0U; k < N; k++)
			carr[k] = alpha_c<T>();
		break;
	case kind::vowels:
		for (auto k = 0U; k < N; k++)
			carr[k] = vowels_c<T>();
		break;
	case kind::alpha_vowels_digits:
		for (auto k = 0U; k < N; k++)
		{
			carr[k] = alpha_c<T>();
			k++;
			if (k > N)
				break;
			carr[k] = vowels_c<T>();
			k++;
			if (k > N)
				break;
			carr[k] = digits_c<T>();
		}
		break;
	default: // kind::alpha_vowels:
		for (auto k = 0U; k < N; k++)
		{
			carr[k] = alpha_c<T>();
			k++;
			if (k > N)
				break;
			carr[k] = vowels_c<T>();
		}
	};
	carr[N] = '\0';
	return carr;
}
} // namespace dbj_tree_research
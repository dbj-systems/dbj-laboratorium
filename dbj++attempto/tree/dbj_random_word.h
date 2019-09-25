#pragma once
#include <array>
#include <algorithm>
#include <string_view>
#include <ctime>

/*
produce radnom words but just a bit more normal looking
or not, or just numbers, or just vowels ... whatever you like
*/
namespace dbj_research
{

	using namespace std;
	using namespace std::string_view_literals;

	template <typename CT> struct meta_data { };

	template < > struct meta_data<char>
	{
		constexpr static auto digits{ "0123456789"sv };
		constexpr static auto alpha{ "bcdfghjklmnpqrstvwxyz"sv }; // no vowels
		constexpr static auto vowels{ "aeiou"sv };
	};

	template < > struct meta_data <wchar_t>
	{
		constexpr static auto digits{ L"0123456789"sv };
		constexpr static auto alpha{ L"bcdfghjklmnpqrstvwxyz"sv }; // no vowels
		constexpr static auto vowels{ L"aeiou"sv };
	};

	template<typename SEQ>
	constexpr size_t top_index (SEQ const & sv_) { return sv_.size() - 1U; }

	// good enough for the purpose
	inline auto random = [](size_t max_val, size_t min_val = 1)
		-> std::size_t {
		static auto initor = []() {
			std::srand((unsigned)std::time(nullptr));
			return 0;
		}();
		return ::abs(int(min_val + std::rand() / ((RAND_MAX + 1u) / max_val)));
	};

	constexpr inline size_t size_one = size_t(1U);

	template <typename CT>
	inline auto random_digit = []() constexpr->CT
	{
		return meta_data<CT>::digits[random( top_index( meta_data<CT>::digits) ) ];
	};

	template <typename CT>
	inline auto random_alpha = []() constexpr -> CT
	{
		return meta_data<CT>::alpha[ random( top_index( meta_data<CT>::alpha) ) ];
	};

	template <typename CT>
	inline auto random_vowel = []() constexpr -> CT
	{
		return meta_data<CT>::vowels[random( top_index( meta_data<CT>::vowels) ) ];
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
		make random words of the "kind" enum above allows
		default is leters and vowels
		to make things simple and fast
		use std::array as return type
		*/
	template <typename T, size_t L>
	constexpr inline array<T, L> const&
		random_word(array<T, L>& carr, kind which_ = kind::alpha_vowels)
	{
		std::size_t N = L - 1;

		switch (which_)
		{
		case kind::digits:
			for (auto k = 0U; k < N; k++)
				carr[k] = random_digit<T>();
			break;
		case kind::alpha:
			for (auto k = 0U; k < N; k++)
				carr[k] = random_alpha<T>();
			break;
		case kind::vowels:
			for (auto k = 0U; k < N; k++)
				carr[k] = random_vowel<T>();
			break;
		case kind::alpha_vowels_digits:
			for (auto k = 0U; k < N; k++)
			{
				carr[k] = random_alpha<T>();
				k++;
				if (k > N)
					break;
				carr[k] = random_vowel<T>();
				k++;
				if (k > N)
					break;
				carr[k] = random_digit<T>();
			}
			break;
		default: // kind::alpha_vowels:
			for (auto k = 0U; k < N; k++)
			{
				carr[k] = random_alpha<T>();
				k++;
				if (k > N)
					break;
				carr[k] = random_vowel<T>();
			}
		};
		carr[N] = T('\0');
		return carr;
	}

	DBJ_TEST_UNIT(random_words) 
	{
		constexpr auto word_length = 8 ;
		std::array<char, word_length>  word;

		//using dbj::console::print;
		using ::dbj::console::PRN; // .printf;

		DBJ_REPEAT(4) 
		{
			auto rword = random_word(word).data();
			PRN.printf("\n%3d: Random Word: '%s'", _dbj_repeat_counter, rword );
		}
	}

} // namespace dbj_research
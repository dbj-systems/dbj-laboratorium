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
		make random words of the "kind" as enum above allows
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

	/*
	 thereis no point of this as for() can not be 
	 compile time affair

	template<size_t N, typename CALLABLE_, typename ... Args >
	void repeat (CALLABLE_ const & fun_ )
	{
		constexpr std::array< bool, N>  sequence{};

		for ([[maybe_unused]] auto& el : sequence) {
			fun_( Args ... );
		}
	}
	*/



	DBJ_TEST_UNIT(random_words) 
	{
		constexpr auto word_length = 8 ;
		std::array<wchar_t, word_length>  word;

		/*
		macro is superior solution
		block is just a block
		_dbj_repeat_counter is available with no stunts
		*/
		DBJ_REPEAT(4) {
			::std::wprintf(L"\n %Z \t Random Word\t '%s'", _dbj_repeat_counter, random_word(word).data());
		}
		DBJ_REPEAT(2) {
			::std::wprintf(L"\n random word:\t '%s'", random_word(word).data());
		}
	}

} // namespace dbj_research
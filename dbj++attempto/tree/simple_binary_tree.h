// leet_binary_tree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "dbj_random_word.h"

namespace dbj_tree_research
{
using namespace ::std;

#pragma region quick utils 
/*
binary_tree_sorted_type does not accept lambdas when definning it
*/
struct s2s final {
	const char* operator () (string& v) { return v.c_str(); }
};

inline auto narrow_to_wide_string = [](string const& node_value_) -> wstring {
	return { node_value_.begin(), node_value_.end() };
};

inline auto random = [](size_t max_val, size_t min_val = 1)
-> std::size_t {
	_ASSERTE(min_val < max_val);
	static auto initor = []() {
		std::srand((unsigned)std::time(nullptr));
		return 0;
	}();
	return ::abs(int(min_val + std::rand() / ((RAND_MAX + 1u) / max_val)));
};

struct num_to_str final
{
	std::array<char, 0xFF> str{ { char(0) } };
	static const std::errc not_posix_error = std::errc();

	template< typename N>
	char const* operator() (N const& number_) noexcept
	{
		static_assert(is_arithmetic_v<N>);
		if (auto [p, ec] = std::to_chars(str.data(), str.data() + str.size(), number_);
			ec == not_posix_error) {
			return str.data();
		}
		else {
			return nullptr;
		}
	}
}; // num_to_str

#pragma endregion

#pragma region dbj bst node
template <
	typename T,
	/* less then */
	typename Comparator_functor,
	/* string reprezentation of the value */
	typename String_functor
>
class binary_tree_sorted_type final
{
	T val;
	binary_tree_sorted_type* left{};
	binary_tree_sorted_type* right{};

	Comparator_functor	comparator;
	String_functor		stringizer;

public:
	using type = binary_tree_sorted_type;
	using value_type = T;

	binary_tree_sorted_type(value_type const& x) noexcept : val(x) {}

	// self erasing
	~binary_tree_sorted_type() {
		if (left != nullptr) {
			delete left;
			left = nullptr;
		}
		if (right != nullptr) {
			delete right;
			right = nullptr;
		}
	}

	void append(const value_type& new_data)
	{
		value_type this_data = this->val;

		if (comparator(new_data, this_data)) // need to do left append
		{
			if (left != nullptr)
				left->append(new_data);
			else
				left = new type(new_data);
		}
		else
		{
			if (right != nullptr)
				right->append(new_data);
			else
				right = new type(new_data);
		}
	}

	void pretty_print
	(string prefix = u8"", bool isLeft = true)
	{
		if (right)
			right->pretty_print(prefix + (isLeft ? u8"│   " : u8"    "), false);

		std::printf(u8"%s%s\n",
			(prefix + (isLeft ? u8"└── " : u8"┌── ")).c_str(),
			stringizer(this->val)
		);

		if (left)
			left->pretty_print(prefix + (isLeft ? u8"    " : u8"│   "), true);
	}


	/* if callback returns false, processing stops */
	template< typename callable_object >
	void pre_proc(callable_object cb_) {
		if (!cb_(this->val)) return;
		if (right) right->pre_proc(cb_);
		if (left) left->pre_proc(cb_);
	}

	template< typename callable_object >
	void in_proc(callable_object cb_) {
		if (right) right->in_proc(cb_);
		if (!cb_(this->val)) return;
		if (left) left->in_proc(cb_);
	}

	template< typename callable_object >
	void post_proc(callable_object cb_) {
		if (right) right->post_proc(cb_);
		if (!cb_(this->val)) return;
		if (left) left->post_proc(cb_);
	}

}; // tree node

#pragma endregion

DBJ_TEST_UNIT(simple_tree_test_very_elegant_printing_algo)
{
	constexpr static auto word_length = 7U;
	constexpr static auto words_count = 9U;

	auto test_lambda = [&](dbj_research::kind word_kind, string prompt) {

		using btree_sorted_words = binary_tree_sorted_type < string, std::less<string>, s2s > ;
		btree_sorted_words root = btree_sorted_words("root");

		array<char, word_length> word_{ {0} };

		for (int k = 1; k < words_count; k++)
		{
			root.append(string(
				dbj_research::random_word(word_, word_kind).data()));
			word_.fill(0);
		}

		::std::printf( "\n%s\n\n", prompt.c_str());

		root.pretty_print( );
	}; // test

#define DRIVER(x) test_lambda(x, _CRT_STRINGIZE(x) )
	// also does the start-up
	DRIVER(dbj_research::kind::alpha);
	DRIVER(dbj_research::kind::alpha_vowels);
	DRIVER(dbj_research::kind::alpha_vowels_digits);
	DRIVER(dbj_research::kind::digits);
	DRIVER(dbj_research::kind::vowels);
#undef DRIVER
	}

#pragma region testing dbj bst with numbers
// legal_call_back
bool by_ten(int& v) { v *= 10; return true; };
bool print(int& v) { printf(" %d ", v); return true; };

DBJ_TEST_UNIT(testing_dbj_bst_with_numbers)
	{
		dbj::win32::syscall("@chcp 65001");

		using binary_sorted_tree = binary_tree_sorted_type< int, std::less<int>, num_to_str >;
		binary_sorted_tree root = binary_sorted_tree(6);

		//for (int k = 1; k < 0xF; k++)
		//{
		//	root.append( binary_sorted_tree::value_type( random(size_t(0xFF))) );
		//}

		root.append(1);
		root.append(9);
		root.append(2);
		root.append(8);
		root.append(3);
		root.append(7);
		root.append(4);
		root.append(5);

		std::printf("\n");
		root.pretty_print();
		std::printf("\nPRE PROC\t");
		root.pre_proc(print);
		std::printf("\nIN PROC\t\t");
		root.in_proc(print);
		std::printf("\nPOST PROC\t");
		root.post_proc(print);
		std::printf("\n");
	}
#pragma endregion
} // namespace dbj_tree_research
// leet_binary_tree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "dbj_random_word.h"

namespace dbj_tree_research
{
using namespace std;

// Definition for a binary tree node.
template <typename T>
struct tree_node_t final
{
	using type = tree_node_t;
	using value_type = T;

	T val;

	tree_node_t *left{};
	tree_node_t *right{};

	tree_node_t(value_type const &x) noexcept : val(x) {}

	// self erasing structure?
	~tree_node_t() {
		if (left != nullptr) {
			delete left;
			left = nullptr;
		}
		if (right != nullptr) {
			delete right;
			right = nullptr;
		}
	}

	// append new node
	// smaller data on the left
	// usage :
	// for (int j = 0; j < countof_(int_arr); ++j)
	//	root.append(int_arr[j]);

	void append(const value_type &new_data)
	{
		value_type this_data = this->val;

		// no duplicates
		if (new_data == this_data)
			return;

		if (new_data < this_data) // need to do left append
		{
			if (left != nullptr)
				left->append(new_data);
			else
				left = new type(new_data);
		}
		else
		{ //need to do right append
			if (right != nullptr)
				right->append(new_data);
			else
				right = new type(new_data);
		}
	}

	/*
	template argument is function pointer of a function
	that transforms T to string
	*/
	template <string (*as_string_)(value_type const &)>
	static void tree_pretty_print(
		type *node, string prefix = "", bool isLeft = true)
	{

		if (node == nullptr)
			return;

		if (node->right)
		{
			tree_pretty_print<as_string_>(node->right, prefix + (isLeft ? "│   " : "    "), false);
		}

		printf("%s", (prefix + (isLeft ? "└── " : "┌── ") + as_string_(node->val) + "\n").c_str());

		if (node->left)
		{
			tree_pretty_print<as_string_>(node->left, prefix + (isLeft ? "    " : "│   "), true);
		}
	}
}; // tree node

DBJ_TEST_UNIT ( simple_tree_test_very_elegant_printing_algo )
{
	system("chcp 65001"); // utf-8 codepage!

	constexpr static auto word_length = 7U;
	constexpr static auto words_count = 9U;

	auto test_lambda = [&](kind word_kind, string_view prompt) {
		auto stocp = [](string const &s_) { return s_; };
		using bst = tree_node_t<string>;
		bst *root = new bst("root");

		array<char, word_length> word_{{0}};

		for (int k = 1; k < words_count; k++)
		{
			root->append(string(
				random_word(word_, word_kind).data()));
			word_.fill(0);
		}
		printf("\n%s\n\n", prompt.data());
		bst::tree_pretty_print<stocp>(root);
	}; // test

#define DRIVER(x) test_lambda(x, #x)
	// also does the start-up
	DRIVER(kind::alpha);
	DRIVER(kind::alpha_vowels);
	DRIVER(kind::alpha_vowels_digits);
	DRIVER(kind::digits);
	DRIVER(kind::vowels);
#undef DRIVER
}

} // namespace dbj_tree_research
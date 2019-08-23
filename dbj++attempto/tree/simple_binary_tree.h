// leet_binary_tree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "dbj_random_word.h"

namespace dbj_tree_research
{
using namespace ::std;

inline auto narrow_to_wide_string = []( string const& node_value_) -> wstring {
	return { node_value_.begin(), node_value_.end() };
};


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
	static void tree_pretty_print(
		type *node, wstring prefix = L"", bool isLeft = true
	)
	{

		if (node == nullptr)
			return;

		if (node->right)
		{
			tree_pretty_print( node->right, prefix + (isLeft ? L"│   " : L"    "), false);
		}

		std::wprintf( L"%s%s\n", 
			   (prefix + (isLeft ? L"└── " : L"┌── ")).c_str() , 
			   narrow_to_wide_string( node->val.c_str() ).c_str()
			);

		if (node->left)
		{
			tree_pretty_print( node->left, prefix + (isLeft ? L"    " : L"│   "), true);
		}
	}
}; // tree node

DBJ_TEST_UNIT(simple_tree_test_very_elegant_printing_algo)
{

	if (0 != system(NULL)) {
		if (-1 == system("chcp 65001"))// utf-8 codepage! 
		{
			switch (errno) {
			case E2BIG: perror("The argument list(which is system - dependent) is too big"); break;
			case ENOENT: perror("The command interpreter cannot be found."); break;
			case ENOEXEC: perror("The command - interpreter file cannot be executed because the format is not valid."); break;
			case ENOMEM: perror("Not enough memory is available to execute command; or available memory has been corrupted; or a non - valid block exists, which indicates that the process that's making the call was not allocated correctly."); break;
			}
		}
	}

	constexpr static auto word_length = 7U;
	constexpr static auto words_count = 9U;

	auto test_lambda = [&](dbj_research::kind word_kind, string prompt) {

		using bst = tree_node_t< string >;
		bst* root = new bst("root");

		array<char, word_length> word_{ {0} };

		for (int k = 1; k < words_count; k++)
		{
			root->append(string(
				dbj_research::random_word(word_, word_kind).data()));
			word_.fill(0);
		}

		::std::wprintf( L"\n%S\n\n", prompt.c_str());

		bst::tree_pretty_print( root );
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
} // namespace dbj_tree_research
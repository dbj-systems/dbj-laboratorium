#pragma once
#include "pch.h"

#ifdef DBJ_TESTING_EXISTS


namespace {

	/*
	An example of what some are calling 'comfortable API'
	*/
	DBJ_TEST_CASE(dbj::FILELINE(__FILE__, __LINE__, ": simple binary tree test"))
	{
		using std::wstring;
		typedef typename dbj::tree::BinaryNode<wstring> BinaryNodeT;

		auto rooty = BinaryNodeT::make_node(L"ROOT");
		(*rooty) << L"Minnie" << L"Moonnie" << L"Tiny" << L"Shiny" << L"Galaxy";
		rooty->insert(L"And", L"Variadic", L"Insert", L"Also", L"Available");
		dbj::print("\n", dbj::treeprint::binary_tree_to_string(rooty.get()), "\n");

		auto printer = [](auto x) {
			DBJ_ASSERT(x);
			dbj::print("\n", dbj::conv::to_wstring(x->data()));
			return true;
		};

		dbj::print("\nProcess then visit left and right: ");
		preorder(rooty.get(), printer);
	}
	/*
	Test on an example tree of BinaryNode<wstring>
	*/
	DBJ_TEST_CASE(dbj::FILELINE(__FILE__, __LINE__, ": binary tree visitors test"))
	{
		using std::wstring;
		constexpr wchar_t nl{ L'\n' };

		/*
		this lambda is our visitor processor
		it can be any lambda, function or functor that receives pointer
		to BinaryNode<> instance and returns bool.
		false	retval from the processor will stop the processing.

		this visitor prints the node data
		*/
		auto printer = [](auto x) {
			DBJ_ASSERT(x);
			dbj::print("\t", dbj::conv::to_wstring(x->data()));
			return true;
		};


		typedef typename dbj::tree::BinaryNode<int> BinaryNodeT;
		auto root = BinaryNodeT::make_node(0);

		(*root) << 1 << 2 << 3 << 4 << 5;

		root->insert(10, 9, 8, 7, 6);

		dbj::print("\nProcess then visit left and right: \n");
		preorder(root.get(), printer);
		dbj::print("\nVisit left, Process then  visit right: \n");
		inorder(root.get(), printer);
		dbj::print("\nVisit left, visit right then Process: \n");
		postorder(root.get(), printer);

		dbj::print("\n", dbj::treeprint::binary_tree_to_string(root.get()), "\n");
	}

} // testing anon space
#endif // DBJ_TESTING_EXISTS

#define DBJVERSION __DATE__ __TIME__
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION ) 
#undef DBJVERSION
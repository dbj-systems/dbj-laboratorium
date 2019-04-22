#pragma once
//#include "pch.h"
//#include <functional>
//#include <dbj++/core/dbj++core.h>

namespace dbj_ref_node {

	template<typename T> struct Node;
	template<typename T> struct Node {

		using type = Node;
		using value_type = T;

		// using link = std::reference_wrapper<Node>;
		using link = dbj::ORW<type>;

		constexpr Node(T data_arg = 0) noexcept 
			: next_({ *this })
			, data_(data_arg == 0 ? '?' : data_arg)
		{}

		bool is_selfref() const noexcept {
			return (this == &(*next_).get() );
		}

		type & next() noexcept { return (*next_).get();	}
		value_type & data() noexcept { return data_; }

		T    data_;
		link next_;
	};

	using demo_node = Node<char>;
	// for those who feel safe seeing the END
	inline const demo_node END(127);


	DBJ_TEST_UNIT( dbj_ref_node )
	{
		demo_node A('A');
		demo_node B('B');
		demo_node C('C');
		DBJ_ATOM_TEST(A.is_selfref() == B.is_selfref() == C.is_selfref());

		A.next() = B;  B.next() = C;

		DBJ_ATOM_TEST(!A.is_selfref() && !B.is_selfref());
		DBJ_ATOM_TEST(C.is_selfref());

		DBJ_ATOM_TEST('A' == A.data());
		DBJ_ATOM_TEST('B' == A.next().data());
		DBJ_ATOM_TEST('C' == A.next().next().data());

		// C.next == C

		C.next() = END;
	}
}
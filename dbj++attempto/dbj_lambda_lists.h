#pragma once
#include "pch.h"

#pragma region LAMBDA LISP

namespace dbj_samples {
	/*
	The key abstraction : list lambda
	*/
	auto llist = [&](auto ...xs) {
		/*return value is lambda */
		return [=](
			/* whose single argument is lambda */
			auto proc_lambda
			) {
			/* which is called with variadic arguments parameter pack
			from the list() call stack
			*/
			return proc_lambda(xs...);
		};
	};
		// ll == "list lambdas" or "lambda lists"
		namespace ll {

		/*
		to understand it all:

		auto l123 = list(1,2,3) // returns lambda --> [=](auto proc_lambda) { return proc_lambda(xs...); };

		head(l123) //  head() is given that lambda as argument
		inside head() calls into that lambda -- see it now above once more
		thus head() does -- l123()
		agument to l123() is lambda defined in the call to it as argument
		(!critical detail here!) that lambda receives xs... that is saved on the
		argument stack of the list() call previously -- see the list() above

		now try to follow all of this in the debuger
		*/
		auto head = [&](auto list_lambda) {
			return list_lambda(
				/*
				this is given as proc_lambda() to the result lambda od the list()
				arguments are the original variadic arguments  of the list()
				*/
				[=](auto first, auto ...rest) {
				/*
				we return list of one element so that users of head() and tail()
				can use  then both in a symetrical manner
				see the usage of print() for example
				*/
				return llist(first);
			}
			);
		};

		// returns list() lambda
		auto tail = [&](auto list_lambda) {
			return list_lambda([=](auto first, auto ...rest) { return llist(rest...); });
		};

		auto length = [&](auto list_lambda) {
			return list_lambda([=](auto ...z) { return sizeof...(z); });
		};

		/* dbj added 
		   use with namespace adornment
		*/
		auto tuple = [&](auto list_lambda) {
			// 
			return list_lambda(
				[=](auto ...args) {
				return std::make_tuple(args...);
			}
			);
		};
} // list 
} // dbj_samples

DBJ_TEST_UNIT(_dbj_lambda_lists_) {

	using namespace dbj_samples;
	auto my_list = llist(1, '2', "3", false, 13.0f); 
	// return lambda() internal to list()
	// explanation
	{
		auto return_first = [&](auto first, auto ... rest) { return first; };
		auto first = my_list(return_first); // returns 1
		auto first_as_a_list = llist(my_list(return_first)); // returns (1)
	}
	// encapsulated implementation usage
	auto my_head = ll::head(my_list); // returns list of one element -- list(1)
	auto my_tail = ll::tail(my_list); // returns list('2', "3", false, 13.0f)
	/*
	after playing with lambda lists we print what we need to print
	we use tuple() because dbj::console::print() knows how to print touples
	but not lambda lists
	*/
	dbj::console::print("\nllist    : ", ll::tuple(my_list));
	dbj::console::print("\nIt's head: ", ll::tuple(my_head));
	dbj::console::print("\nIt's tail: ", ll::tuple(my_tail));

	 dbj::console::print("\nLength of llist --> ", DBJ_NV(ll::length(my_list)) );

	 dbj::console::print("\n\ndbj::print can print pairs too, here is one: ");

	 auto pofs = DBJ_TEST_ATOM (
		 std::make_pair("Pair of ", " strings ")
	 );
}
#pragma endregion 

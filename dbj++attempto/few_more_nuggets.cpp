// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"

namespace dbj::samples{ // beware of anonymous namespace

	using namespace std; // beware of this

	// handling aguments param pack when all the 
	// arguments are of the same type
	auto args_initlist = [](auto && ... args)
	{
		if constexpr(1 > (sizeof... (args))) {
			return{};
		}
		else
		{
			using namespace std;
			// here we transform arguments parameter pack to init list
			// aka "list initialization" 
			// https://en.cppreference.com/w/cpp/language/list_initialization
			// NOTE: since C++17 more than one element in the init list can be deduced
			// only by copy declaration
			// this is thus error -> auto	arg_list{ args ... };
			// and this is not
			auto	arg_list = { args ... };
			// the type of the init list instance
			using	arg_list_type = decltype(arg_list);
			// the type of the elements in the init list instance
			using   arg_list_value_type = typename arg_list_type::value_type;
			/*
				init list is a kind of a range
				so we can handle its elements in an range for loop

				for (element && : arg_list ) {  ... do something with element ...  }
				*/
			return arg_list;
		}
	};


	namespace inner {

		using namespace std;

		template < class Tuple, size_t... I>
		inline void tup_print(Tuple&& t, index_sequence<I...>)
		{
			dbj::console::print(get<I>(forward<Tuple>(t))...);
		}
	}  // namespace inner

	template <class Tuple>
	inline void tup_print(Tuple&& t)
	{
		using namespace std;
		constexpr size_t tup_size = tuple_size_v<remove_reference_t<Tuple>>;
		inner::tup_print(
			forward<Tuple>(t), make_index_sequence<tup_size>{}
		);
	}

	// handling aguments param pack when 
	// arguments are of different types
	// template< typename ... Args >
	auto args_tuple = [](auto && ... args)
	{
		if constexpr(1 > (sizeof... (args))) {
			return{};
		}
		else
		{
			using namespace std;
			// here we transform arguments parameter pack to tuple
			auto	tup_list = make_tuple(args ...);
			// the type of the tuple instance
			using	arg_list_type = decltype(tup_list);

			/*
			tuple is not a range
			dbj::console::print(tup_list);
			see here how to get to the tuple elements
			https://en.cppreference.com/w/cpp/utility/tuple/get
			*/
			return tup_list;
		}
	};

	DBJ_TEST_UNIT(param_pack_to_initlist)
	{

		auto init_list =  DBJ_TEST_ATOM( args_initlist(1, 2, 3) );

		// this does not compile, wot now?
		// args_initlist(1, 2.34, true, L"Abra", " Ka", " Dabra!");
		//
		auto tup_ =  DBJ_TEST_ATOM(
			args_tuple(1, 2.34, true, L"Abra", " Ka", " Dabra!")
			) ;
		auto &&[a, b, c, d, e, f] = tup_;
	}

	template < typename T >
	constexpr std::decay_t<T>
		to_base_value(const T & value_)
	{
		using DT =  std::decay_t<T>;
		return DT{};
	}
/* */
	template < typename T >	constexpr T to_base_value( T && value_ ) = delete;  
/* */

	DBJ_TEST_UNIT(static_assert_or_delete)
	{
		constexpr int rr = 42;
		auto r1 = to_base_value(rr);
		const int arr[]{1,2,3};
		auto r2 = to_base_value(arr);
		// int arr2[] {1, 2, 3}; auto r3 = to_base_value(arr2);

		auto r4 = to_base_value("ABC");

		// auto r6 = to_base_value(nullptr);
		// auto r5 = to_base_value( std::forward<int>(42)) ;
	}

} // anon ns
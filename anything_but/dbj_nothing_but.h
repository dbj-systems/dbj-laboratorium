#pragma once
/*
	Type Handle to Avoid Implicit Conversions in standard C++

	Copyright(C) 2019 Dušan B. Jovanović (dbj@dbj.org)

	This program is free software : you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see < https://www.gnu.org/licenses/>.
*/

#include <type_traits>

namespace dbj {
	namespace util {
		using namespace std;

		/*
		avoid implicit conversions to/ from type T
		by handling it through this class

		terminology: type X is any other type but T
		*/
		template<typename T>
		struct nothing_but final
		{
			static_assert(false == std::is_array_v<T>, "can not deal with arrays");

			using type = nothing_but;

			nothing_but() : val_(T{}) {}

			nothing_but(T const & t_) : val_(t_) {}
			type & operator = (T const & new_val_)
			{
				val_ = new_val_;
				return *this;
			}

			/* to construct from X is banned */
			template<
				typename X,
				std::enable_if_t<
				false == std::is_same_v<T, X>
				, int> = 0
			>
				nothing_but(X const & x_) = delete;

			/* to assign from X is banned */
			template<
				typename X,
				std::enable_if_t<
				false == std::is_same_v<T, X>
				, int> = 0
			>
				type & operator = (X const & new_val_) = delete;

			// conversion to T&, but only if not const
			operator T & () { return val_; }

			/* conversion to X is banned */
			template<
				typename X,
				std::enable_if_t<
				false == std::is_same_v<T, X>
				, int> = 0
			>
				operator X & () = delete;

			// as other std class types do
			T & data() const { return (T&)val_; }

		private:
			T val_{};

			// compatibility
			friend bool operator < (type const & left_, type const & right_)
			{
				return left_.val_ < right_.val_;
			}
		};

	} // util
} // dbj

#pragma warning( push )
#pragma warning( disable : 4189 )
// warning C4189 : unused local variables
//
//DBJ_TEST_SPACE_OPEN(nothing_but)
//
//using dbj::util::nothing_but;
//
//void  uncertain_arg_type(size_t Size) { (void)&Size; }
//
//void  exact_arg_type(nothing_but<size_t> Size) { (void)&Size; }
//
//DBJ_TEST_UNIT(dbj_string_c_lib)
//{
//	using dbj::util::nothing_but;
//
//	{ // constnes
//		const int cc = 42;
//		const int cb = cc;
//		nothing_but<int> const & sci1 = cc;
//		nothing_but<int> const sci2 = sci1; // ok
//		const nothing_but<int> sci3 = sci1; // ok
//
//		// data() is for a Peeping Tom's
//		// yes can do
//		bool peeping_tom = sci1.data();
//		// no can do -- bool peeping_tom = sci1;
//
//		wprintf(L"%d", sci1.data()); // ok
//
//		//sci2 = sci1;
//		//const int ii1 = sci1;
//		//const int ii2 = static_cast<int>(sci1);
//		//const int * ii3 = const_cast<int *>(&(sci1));
//		const int & ii3 = (const int &)(sci1); // ok
//		int & ii4 = (int &)(sci1); // ok
//	}
//	{ // basic tests
//		nothing_but<int> si1 = 42;
//		si1 = 42;
//		nothing_but<int> si2 = 13;
//		nothing_but<int> si22{ 13 };
//		// nothing_but<int> si22{true};
//		//nothing_but<int> si3 = true ; 
//		//si3 = true;
//		//nothing_but<int> si4 = '$' ;
//		//si4 = '$';
//		//nothing_but<int> si5 = 2.7 ;
//		//si5 = 2.7;
//		//nothing_but<int> si6 = size_t(BUFSIZ) ;
//		//si6 = size_t(BUFSIZ);
//
//		si1 = si2; (void)(int(si1) == int(si2));
//
//		int j = 9;
//		nothing_but<int *> sip1 = &j;
//		nothing_but<char const *> scc1 = "OK";
//
//		char name[] = "c++";
//		// scc1 = name;
//	}
//	{ // moving
//		nothing_but<char const *> scc1 = "OK";
//
//		auto lambda = [](nothing_but<char const *> str)
//			-> nothing_but<size_t>
//		{
//			return std::strlen(str);
//		};
//
//		assert(3 == lambda("123"));
//	}
//	{ // compatibility
//		// std::vector<nothing_but<bool>> bv{ true, 1 , true };
//		std::vector<nothing_but<bool>> bv{ true, true , true };
//
//		nothing_but<bool> bb = bv[1];
//		bool kb = bb; // OK
//		//int k = bb;
//		//int j = bv[1];
//
//		std::map< nothing_but<bool>, nothing_but<int> > bm;
//
//		bm[true] = 1;
//		// bm[true]  = 2.3;
//		bm[false] = 0;
//		// bm[false] = 0.9;
//	}
//	{
//		using not_bool = std::negation<int>;
//		// wprintf(L"%S", typeid(not_bool::type).name());
//	}
//}
//
////DBJ_TEST_UNIT(string_ctor_solution) {
////
////	using  enforced_size_t = dbj::util::nothing_but<size_t>;
////	/*
////	this is the ctor:
////
////	basic_string( const size_type _Count, const _Elem _Ch)
////	// construct from _Count * _Ch
////
////	and this is the bug (warnings issued)
////	*/
////	std::string  stars_not('*', 0xFF);
////	/*
////	it should be:
////	*/
////	std::string  stars(0xFF, '*');
////	
////
////	// all this works with some or no warnings but should or must not
////	uncertain_arg_type('*'); uncertain_arg_type(true); uncertain_arg_type(2.7);
////	uncertain_arg_type(BUFSIZ); //  ok because of implicit conversion
////
////	/* mistakes can not be made because of the argument type
////
////	exact_arg_type('*'); exact_arg_type(true); exact_arg_type(2.7);
////	*/
////
////	/* ok let's make that argument of the exactly required type
////	no can do as BUFSIZ type is *not* size_t
////	enforced_size_t bufsiz = BUFSIZ ;
////	*/
////
////	size_t bufsiz = BUFSIZ; // implicit conversion
////	exact_arg_type(bufsiz); // yes can do size_t can be assigned to enforced_size_t
////}
//
//DBJ_TEST_SPACE_CLOSE

#pragma warning( pop ) 

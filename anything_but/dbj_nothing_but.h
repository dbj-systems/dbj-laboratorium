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
#include <iosfwd>

namespace dbj {

	namespace util {
		using namespace std;

		/*
		On C++ types, start from here: https://en.cppreference.com/w/cpp/types/is_fundamental
		Bellow one can see what type we currently allow.
		*/
		template< class T >
		struct is_ok_for_nothing_but
			: std::integral_constant<
			bool,
			std::is_union<T>::value ||
			std::is_class<T>::value ||
			std::is_enum<T>::value ||
			std::is_pointer<T>::value ||
			std::is_arithmetic<T>::value 
			> {};

		/*
		avoid implicit conversions to/ from type T
		by handling it through this class

		terminology: type X is any other type but T
		*/
		template<typename T>
		struct nothing_but final
		{
			static_assert(is_ok_for_nothing_but<T>::value, "\n\ndbj::util::nothing_but can not deal with this type.\n");

			using type = nothing_but;

			// default ctor makes default T
			nothing_but() : val_(T{}) {}
			// moving
			nothing_but(nothing_but && other_) : val_(std::move(other_.val_)) { }
			type & operator = (nothing_but && other_)
			{
				this->val_ = std::move(other_.val_);
				return *this;
			}

			 //to convert or assign from T is allowed
			 //to move from T is allowed
			nothing_but(T && t_) = delete; // : val_(std::move(t_)) { }
			type & operator = (T && new_val_) = delete; // { val_ = std::move(new_val_); return *this; }

			  //to convert or assign from T is allowed
			  // by copying
			nothing_but(T const & t_) : val_(t_) { }
			type & operator = (T const & new_val_)
			{
				val_ = new_val_;
				return *this;
			}

			/* to construct from X is banned */
			template< typename X, std::enable_if_t<false == std::is_same_v<T, X>, int> = 0>
				nothing_but(X & x_) = delete;

			/* to assign from X is banned */
			template<typename X,std::enable_if_t<false == std::is_same_v<T, X>, int> = 0>
				type & operator = (X & new_val_) = delete;

			// conversion to T&, but only if not const
			operator T & () { return val_; }

			/* conversion to X is banned */
			template<typename X,std::enable_if_t<false == std::is_same_v<T, X>, int> = 0 >
				operator X & () = delete;

			// as elsewhere in std 
			// the convention is to provide 'data()' method
			// for users to reach to non const data handled
			T & data() const { return (T&)val_; }

		private:
			T val_{};

			// compatibility
			// to act as element type in some of std:: comtainers
			// class has to provide less than operator
			friend bool operator < (type const & left_, type const & right_)
			{
				return ((left_.val_) < (right_.val_));
			}

			// this means type T has to be compatible too 
			// that is std::ostream & << ( std::ostream, T const & );
			// must be defined and in the scopse
			friend std::ostream & operator << (std::ostream & os_, type const & right_)
			{
				return os_ << right_.val_;
			}
		};

	} // util
} // dbj

// EOF
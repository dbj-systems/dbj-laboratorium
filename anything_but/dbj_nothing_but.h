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

			friend std::ostream & operator << (std::ostream & os_ , type const & right_)
			{
				return os_ << right_.val_;
			}
		};

	} // util
} // dbj

// EOF
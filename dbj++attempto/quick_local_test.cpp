// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"

namespace {
	using namespace std;
	/*
	http://www.open-std.org/pipmail/ub/2016-February/000565.html
	*/
	/// <summary>
	/// C++ "stunt" which perhaps is not. It shows the good usage of std artefacts
	/// based on SFINAE. And it is suffciently limiting function not to be labelled a "stunt"
	/// Both types have to be :
	///  of the same size
	///  trivially copyable
	///  and destructible
	/// This also shows extremely efficient compile-time coding of course.
	/// And. It this is an example where raw pointer are most efficient mechanism
	/// No copying and no moving is invoved here ...
	/// </summary>
	template<typename T, typename U>
	static T * change_object_type(U *p) {
		static_assert(sizeof(T) == sizeof(U));
		static_assert(is_trivially_copyable_v<T> && is_trivially_copyable_v<U>);
		char buff[sizeof(T)];
		memcpy(buff, p, sizeof(T));
		p->~U();
		T *result = new (p) T;
		memcpy(result, buff, sizeof(T));
		return result;
	}

	/// <summary>
	/// Encapsulation of the usage of  change_object_type
	/// </summary>
	/// <param name="f"> float to be magically transformed to int</param>
	/// <returns>the result of the "magic"</returns>
	static int magic_int_float_transformation(float f) {
		/** float is dead, long live the int! */
		int *i = change_object_type<int>(&f);
		return *i;
	}

	/// <summary>
	/// Illustrating the use of Arrays and Functions in C
	/// https://www.cs.uic.edu/~jbell/CourseNotes/C_Programming/Arrays.html
	/// </summary>

	typedef double numarr[];

	// Finds max in the array
	extern "C" static double maxArray(const numarr numbs, int arraySize);

	extern "C"  static int Illustrating_the_use_of_Arrays_and_Functions(void) {

		// here we use the typedef in declaration
		numarr array1{ 10.0, 20.0, 100.0, 0.001 };

		// we can not use the same single dim array in declaring two dim array
		double array2[2][3]{
			{ 5.0, 10.0, 20.0 },
			{ 8.0, 15.0, 42.0 }
		};

		int sizes[2] = { 4, 3 };
		double max1, max2, max3, max4, max5;

		max1 = maxArray(array1, 4);
		max2 = maxArray(array1, sizes[0]);
		// max3 works becuse of how arrays are
		// laid out in memory
		// max3 is thus sum of the second row
		// of the array2
		max3 = maxArray(array2[1], 3);
		// max4 also works becuse of how arrays are
		// laid out in memory
		// max4 is thus sum of the whole array2
		max4 = maxArray(array2[0], 6);
		max5 = maxArray(array1, -4);

		dbj::console::print("\nMaximums are ", max1, ", ", max2, ", ", max3
			, ", ", max4, ", ", max5);

		return 0;
	}

	/// <summary>
	/// Function to find the maximum in an array of doubles
	/// Note the use of the keyword "const" to prevent changing array data
	/// </summary>
	/// <param name="numbs">array argument </param>
	/// <param name="arraySize">user defined size of the array</param>
	/// <returns></returns>
	extern "C" static double  maxArray(const numarr numbs, int arraySize) {

		int i;
		double max;

		if (arraySize <= 0) {
			return 0.0;
		}

		max = numbs[0];

		for (i = 1; i < arraySize; i++)
			max = (numbs[i] > max) ? numbs[i] : max;

		return max;

	}

	/// <summary>
	/// tuple is collection of different types
	/// vector elementes are all of the same type
	/// there is no much point transforming vector to tuple
	/// </summary>
	static void test_vector_to_touple() {
		vector<int> vint{ 1, 2, 3 };
		auto tpl0 = dbj::util::seq_tup<vector<int>, 3>(vint);

		int vintage[]{ 1, 2, 3 };
		auto tpl1 = dbj::util::seq_tup(vintage);
	}

	/// <summary>
	/// c++ 17 generic lambdas have issues
	/// with required types of auto arguments
	/// in c++20 this will be fixed with new
	/// lambda arguments template declaration syntax
	/// </summary>
	namespace required_types
	{
		template<typename RQ>
		inline auto  is_required_type = [](const auto & v_ = 0) constexpr -> bool
		{
			using T = std::decay_t< decltype(v_) >;
			return std::is_same<T, RQ>();
		};


		inline auto is_uint64 = [](const auto & v_ = 0) constexpr -> bool
		{
			return is_required_type<std::uint64_t>(v_);
		};

	} // required_types

	using namespace required_types;

	inline auto tv = [](const char prompt[] = "", const auto & value) -> void {
		// std::cout << prompt << "\ntype:\t" << typeid(decltype(value)).name() << "\nvalue:\t" << value;
		dbj::console::print(prompt);
		auto rez[[maybe_unused]] = DBJ_TEST_ATOM(value);
	};


	inline auto make_double_value = [](auto value)
	{
		if constexpr (is_uint64(value)) {
			tv("\n\nDoubling required type (std::uint_64):", value);
			return value + value;
		}

		tv("\n\nWill try to double 'illegal' type", value);
		return value + value;
	};

	//------------------------------------------------------------------------------
	// https://stackoverflow.com/questions/13401716/selecting-a-member-function-using-different-enable-if-conditions/50412114#50412114
	namespace so {
		template<typename T>
		struct Point
		{
			static_assert (
				std::is_same<T, std::uint64_t>() ||
				std::is_same<T, std::float_t>()
				);

			typedef T value_type;

			Point() = default;

			auto data() const {
				return std::array<T, 2>{ this->x, this->y };
			}

			template<typename U>
			const Point & assign(U x_, U y_)
			{
				if constexpr (std::is_same<T, U>()) {
					worker(x_, y_);
				}
				// else 
				worker(
					static_cast<T>(x_),
					static_cast<T>(y_)
				);
				return *this;
			}

			template<typename U>
			const Point & assign(Point<U> p_)
			{
				if constexpr (std::is_same<T, U>()) {
					worker(p_.x, p_.y);
				}
				// else 
				worker(
					static_cast<T>(p_.x),
					static_cast<T>(p_.ys)
				);
				return *this;
			}


		private:

			mutable T x{}, y{};

			void worker(T x_, T y_)
			{
				x = x_; y = y_;
			}

			friend
				Point mid(const Point & p1, const Point & p2) {
				Point retval;
				if constexpr (std::is_same<value_type, std::uint64_t>()) {
					// we need to transform to integer
					std::uint64_t mid_x = (std::uint64_t)((p1.x + p2.x) / 2);
					std::uint64_t mid_y = (std::uint64_t)((p1.y + p2.y) / 2);

					return retval.assign(mid_x, mid_y);
				}

				return retval.assign(
					static_cast<value_type>((p1.x + p2.x) / 2),
					static_cast<value_type>((p1.y + p2.y) / 2)
				);
			}
		};
	}

	static void limit_on_type() {
		so::Point<std::uint64_t>		pi1, pi2;
		so::Point<std::float_t>	pf1;

		pi1.assign(1234, -2878);
		pi2.assign(-734, 9876);
		pf1.assign(1324.980f, 456.390f);

		auto mid_ = mid(pi1, pi2);

		auto[x, y] = mid_.data();
	}

}
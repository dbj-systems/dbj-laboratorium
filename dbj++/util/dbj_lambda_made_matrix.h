#pragma once
/*
Also at: https://wandbox.org/permlink/hSE7d24YmNIOSVII

Here are matrices made and supported by a single lambda.
(the other one is not necessary)

Two versions: 

1. dynamic -- made at runtime on the heap
2. static  -- made at complie time on the stack

*/
#include <memory>
#include <array>

namespace dbj::lambda_matrix
{
	constexpr unsigned short max_cols = 0xFFFF;
	constexpr unsigned short max_rows = 0xFFFF;
	// fast and dangerous or slow and safe
	constexpr bool check_indexes_on_each_call = false;

	template<typename T>
	auto mtx(unsigned short height_, unsigned short width_)
	{
		static_assert(std::is_arithmetic_v <T>);
		static_assert(std::is_nothrow_constructible_v <T>);

		DBJ_VERIFY(width_ <= max_cols);
		DBJ_VERIFY(height_ <= max_rows);

		return[
			arry = std::make_unique<T[]>(height_ * width_), height_, width_
		]
		(size_t row_, size_t col_) mutable->T&
			{
				// good: we keep the size of each instance
				if constexpr (check_indexes_on_each_call) {
					DBJ_VERIFY(row_ <= height_);
					DBJ_VERIFY(col_ <= width_);
				}
				// arry is moved into here
				return arry[row_ * width_ + col_];
			};
	}

	// the fastest solution
	template<typename T, size_t H, size_t W>
	inline auto fast_mtx() {
		static_assert(std::is_arithmetic_v <T>);
		static_assert(std::is_nothrow_constructible_v <T>);

		constexpr auto height_ = H;
		constexpr auto width_ = W;

		static_assert(width_ <= max_cols);
		static_assert(height_ <= max_rows);

		T arry[H*W]{};

		return [
			arry /*std::array<T,H*W>{{}}*/, height_, width_
		]
		(size_t row_, size_t col_) mutable->T&
		{
			if constexpr (check_indexes_on_each_call) {
				DBJ_VERIFY(row_ <= height_);
				DBJ_VERIFY(col_ <= width_);
			}
			// arry is moved into here
			return arry[row_ * width_ + col_];
		};
	} // fast_mtx

	 /*
	 visitor signature:
	 bool (*) (unsigned short rows_, unsigned short columns_, auto & mtx_, auto const & visitor_);
	 processing stops on false returned
	 */
	inline auto for_each_cell = []
	(unsigned short rows_, unsigned short columns_, auto & mtx_, auto const & visitor_)
	{
		// debug build only
		assert(rows_ <= max_rows);
		assert(columns_ <= max_cols);

		for (auto j = 0; j < rows_; j++)
			for (auto k = 0; k < columns_; k++)
			{
				if (false == visitor_(mtx_(j, k), j, k)) return;
			}
	};

	// a bit more complicated in order to display better layout
	inline auto printer = []( auto HEIGHT, auto WIDTH) {
		return [HEIGHT, WIDTH](auto & val_, short row, short col)
			-> bool
		{
			using ::dbj::console::print;
			static int col_counter_ = 1;
			if ((0 == row) && (0 == col))	print("\n{");
			if (0 == (col % WIDTH))	print("\n\t{");
			// full display: print(" [", row,",",col,"] = ",val_, " ");
			print(" ", val_, " ");
			if (0 == (col_counter_ % WIDTH)) print("}");
			if ((HEIGHT == row + 1) && (WIDTH == col + 1))	print("\n}\n");
			col_counter_++;
			return true;
		};
	};

} // namespace dbj::lambda_matrix

#ifdef DBJ_LAMBDA_MATRIX_TESTING

#include "../console/dbj++con.h"

inline int dbj_lambda_matrix_testing ()
{
	using ::dbj::console::print;
	using namespace dbj::lambda_matrix;
	constexpr auto HEIGHT = 3, WIDTH = 3;

	// see the -- ::dbj::mtx::check_indexes_on_each_call = true/false ;

	//heap based runtime solution
	//type is given at compile time
	//auto mx_1 = mtx<int>(HEIGHT, WIDTH);
	//auto mx_2 = mtx<int>(HEIGHT, WIDTH);
	//auto mx_3 = mtx<int>(HEIGHT, WIDTH);

	auto mx_1 = fast_mtx<int, HEIGHT, WIDTH>();
	auto mx_2 = fast_mtx<int, HEIGHT, WIDTH>();
	auto mx_3 = fast_mtx<int, HEIGHT, WIDTH>();

	// population
	auto put42 = [](auto & val_, auto row, auto cel) -> bool
	{
		noexcept(row, cel); // no use
		val_ = 42;	return true;
	};
	for_each_cell(HEIGHT, WIDTH, mx_1, put42);
	for_each_cell(HEIGHT, WIDTH, mx_2, put42);

	// addition
	auto adder_visitor = [&](auto & val_, auto row, auto cel) -> bool
	{
		noexcept(val_); // no use
		mx_3(row, cel) = mx_1(row, cel) + mx_2(row, cel);
		return true;
	};

	for_each_cell(HEIGHT, WIDTH, mx_3, adder_visitor);

	// display
	auto printer_visitor = printer(HEIGHT, WIDTH);

	print("\nMX 1");
	for_each_cell(HEIGHT, WIDTH, mx_1, printer_visitor);

	print("\nMX 2");
	for_each_cell(HEIGHT, WIDTH, mx_2, printer_visitor);

	print("\nMX 3");
	for_each_cell(HEIGHT, WIDTH, mx_3, printer_visitor);

	return EXIT_SUCCESS;
}

#endif // DBJ_LAMBDA_MATRIX_TESTING

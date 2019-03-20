#pragma once
#include "../dbj_gpl_license.h"
#include "../util/dbj_stack_matrix.h"
#include "../testfwork/dbj_testing.h"
#include "../console/dbj_console_ops.h"

namespace dbj_stack_matrix_testing {

	using namespace ::dbj::arr;
	using ::dbj::console::print;

	template< typename MX, typename T >
	inline void test_mx(T new_val)
	{
		print("\n ID: ", MX::uuid(), ", size: ", MX::weight(), ", rank: ", MX::rank());
		// can update this way -- auto & mx = MX::data() ; mx[1][1] = T(1);
		// or this way -- MX::data(1, 1) = T(1) ;
		// or this way
		MX::for_each(
			[&](typename MX::value_type & val, size_t r, size_t c)
			->bool
		{
			val = new_val++;
			noexcept(r, c); //dummy usage to pacify msvc
			return true; // proceed
		}
		);
		MX::printarr(dbj::console::print);
	}

	template< typename MX>
	inline MX test_mx_arg_retval(MX the_mx)
	{
		// leave the trace
		the_mx.data(
			the_mx.height() - size_t(1),
			the_mx.width() - size_t(1)
		) = typename MX::value_type(1234);
		return the_mx;
	}

	DBJ_TEST_UNIT(dbj_static_matrix_multiplication)
	{
		// for_each() callback type
		// bool(*)(value_type &, size_t, size_t);
		auto filler = [](auto & val_, size_t col, size_t row) -> bool {
			val_ = int(1); // int(col + row);
			return true;
		};
		/*
		  A[n][m] x B[m][p] = R[n][p]
		  stack_matrix<T,R,C,UID> matrix is R x C matrix
		  Cols of A must be the same as height of B
		  R must be A height x B width
		*/
		using A = stack_matrix<int, 0xF, 0xF, DBJ_UID >;
		using B = stack_matrix<int, A::width(), 0xF, DBJ_UID >;
		using R = stack_matrix<int, A::height(), B::width(), DBJ_UID >;

		A::for_each(filler);
		B::for_each(filler);

		stack_matrix_multiply<A, B, R>();
		R::printarr(::dbj::console::print);
	}

	DBJ_TEST_UNIT(using_dbj_static_matrix_as_storage)
	{
		// create storage for 3 int 3 x 3 matrices
		using A = stack_matrix<int, 3, 3, DBJ_UID >;
		using B = stack_matrix<int, 3, 3, DBJ_UID >;
		using R = stack_matrix<int, 3, 3, DBJ_UID >;
		// write into the matrices
		A::data()[0][0] = 42;	A::data()[1][1] = 42;	A::data()[2][2] = 42;
		B::data()[0][0] = 42;	B::data()[1][1] = 42;	B::data()[2][2] = 42;

		// lets asume and imagine our internal multiplication function is 
		// some external lib function
		//	template<typename T, size_t N, size_t M, size_t P>
		//	inline void multiply(T(&a)[N][M], T(&b)[M][P], T(&c)[N][P]);
		// we want to use but with the data kept inside dbj static matrix
		// we can do it in a round-about way
		std::reference_wrapper<int[A::height()][A::width()]> ref_a = std::ref(A::data());
		A::matrix_ref_type mr_a = ref_a;

		std::reference_wrapper<int[B::height()][B::width()]> ref_b = std::ref(B::data());
		B::matrix_ref_type mr_b = ref_b;

		std::reference_wrapper<int[R::height()][R::width()]> ref_r = std::ref(R::data());
		R::matrix_ref_type mr_r = ref_r;

		::dbj::arr::inner::multiply(mr_a, mr_b, mr_r);

		// or in a silly easy way
		::dbj::arr::inner::multiply(A::data(), B::data(), R::data());

	}

	DBJ_TEST_UNIT(native_matrix_using_dbj_arr_space)
	{
		using ::dbj::console::print;

		int A[][3] = { {1,1,1},{1,1,1},{1,1,1} };
		int B[][3] = { {1,1,1},{1,1,1},{1,1,1} };
		int C[][3] = { {0,0,0},{0,0,0},{0,0,0} };

		::dbj::arr::inner::multiply(A, B, C);
		print("\nresult:\n");
		print("\n", C[0]);
		print("\n", C[1]);
		print("\n", C[2]);

	}

	DBJ_TEST_UNIT(dbj_static_matrix) {

		constexpr size_t R = 3, C = 3;

		// dbj static matrix solution
		// NOTE! bellow are two different types 
		// since DBJ_UID produces two different UID's
		// NOTE! do not try or test DBJ_UID in a loop as that
		// makes all the uid's to be the same; as DBJ_UID
		// is replaced with a value only once and first, by a pre-processor
		using mx9a = stack_matrix<int, R, C, DBJ_UID >;
		using mx9b = stack_matrix<int, R, C, DBJ_UID >;

		_ASSERTE(mx9a::uuid() != mx9b::uuid());

		test_mx<mx9a>(0);
		test_mx<mx9b>(100);

		// instances are ok but perfectly
		// redundant in this context
		mx9a mxa = test_mx_arg_retval(mx9a());
		mx9b mxb = test_mx_arg_retval(mx9b());

		mxa.printarr(::dbj::console::print);
		mxb.printarr(::dbj::console::print);
	}
	/*
		This is certainly better memory layout but equaly certainly more
		computation intensive.

		We can now have lambdas return lvalue references, which can make things
		a (much) more user friendly. This could be the fastest solution.
	*/
	namespace dbj::mtx
	{
		constexpr unsigned short max_width = 0xFF;
		constexpr unsigned short max_height = 0xFF;

		template<typename T>
		auto mtx(unsigned short height_, unsigned short width_)
		{
#ifdef _MSC_VER
			static_assert(std::is_arithmetic_v <T>, "\n\nstatic assert in:\n\n" __FUNCSIG__ "\n\n\tOnly numbers please!\n\n");
#else
			static_assert(std::is_arithmetic_v <T>, "\n\nstatic assert in: dbj::mtx::mtx()\t\nOnly numbers please!\n\n");
#endif
			DBJ_VERIFY(width_ <= max_width);
			DBJ_VERIFY(height_ <= max_height);

			return [
				arry = std::make_unique<T[]>(height_ * width_), height_, width_
			]
			(size_t row_, size_t col_) mutable->T&
			{
					DBJ_VERIFY(row_ <= height_);
					DBJ_VERIFY(col_ <= width_ );
				// arry is moved into here
				return arry[row_ * width_ + col_];
			};
		}

		/*
		visitor signature:

		bool (*) (unsigned short rows_, unsigned short columns_,
			auto & mtx_, auto const & visitor_);		

			processing stops on false returned
		*/
		auto for_each_cell = []
		(unsigned short rows_, unsigned short columns_,
			auto & mtx_, auto const & visitor_
		)
		{
			// not extremely usefull
			// we do not know the dimension of the mtx_ 
			DBJ_VERIFY(columns_ <= max_width);
			DBJ_VERIFY(rows_ <= max_height);

			for (auto j = 0; j < rows_; j++)
				for (auto k = 0; k < columns_; k++)
				{
					if (false == visitor_(mtx_(j, k), j, k)) return;
				}
		};
	} // mtx

	DBJ_TEST_UNIT(dbj_lambda_arrays)
	{
		using namespace dbj::mtx;
		using ::dbj::console::print;

		// auto mx_max = mtx<int>(0xFF, 0xFF); // max size

		auto R = 3, C = 3;

		auto mx_1 = mtx<int>(R, C);
		auto mx_2 = mtx<int>(R, C);
		auto mx_3 = mtx<int>(R, C);

		// population
		auto put42 = [](auto & val_, auto row, auto cel) -> bool
		{
			val_ = 42;	return true;
		};
		for_each_cell(R, C, mx_1, put42) ;
		for_each_cell(R, C, mx_2, put42) ;

		// addition
		auto adder_visitor = [&](auto & val_, auto row, auto cel) -> bool
		{
			mx_3(row, cel) = mx_1(row, cel) + mx_2(row, cel);
			return true;
		};

		for_each_cell(R, C, mx_3, adder_visitor);

		// display
			auto printer_visitor = [R,C](auto & val_, short row, short col) 
			{
				using ::dbj::console::print;
				static int col_counter_ = 1;
				if ((0 == row)  && (0 == col) )	print("\ndbj::mtx {");
				if (0 == (col % C ))	print("\n\t{");
					// full display: print(" [", row,",",col,"] = ",val_, " ");
					print(" ",val_, " ");
				if ( 0 == ( col_counter_ % C)) print("}");
				if ((R == row+1) && (C == col+1))	print("\n}\n");
				col_counter_++;
				return true;
			};

		print("\nMX 1");
		for_each_cell(R, C, mx_1, printer_visitor);

		print("\nMX 2");
		for_each_cell(R, C, mx_2, printer_visitor);

		print("\nMX 3");
		for_each_cell(R, C, mx_3, printer_visitor);
	}

} // namespace dbj_stack_matrix_testing 
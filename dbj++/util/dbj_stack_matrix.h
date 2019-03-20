#pragma once

// #include "../win/dbj_uuid.h"
/*
IMPORTANT: Terminology

Matrix has HEIGHT and WIDTH
HEIGHT is made of 0..number of ROWS
WIDTH is made of 0 ... number of COLUMNS

I like to express matrix dimensions as height X width
Because of usage of the term's "col" and "row" to give a meaning to arguments
I preffer to use height and width.

-------------------------------------------------------------------
Why this? Consider ...
-------------------------------------------------------------------

T[][N] would be called "array of array [N] of T" and be an incomplete matrix_type,
while T[][] would be an invalid matrix_type (all except the last dimensions
must have a known size).

T[N][M] is "array [N] of array[M] of T",
while int * [sizeX] is "array [sizeX] of T" where T is a pointer to an int.
Creating dynamically an 2d array works like this:

 // WIDTH++11 onwards -- allocate (with initialization)
	auto array = new double[M][N]();
	delete [] array ;

It will create an array of an allocated-matrix_type int[X][Y].
This is a sort-of-a "hole" in WIDTH++'s matrix_type system, since the ordinary matrix_type system
of WIDTH++ doesn't have array dimensions with run-time sizes (not known at compile time),
thus these are called "allocated types"

I am worried about above being slow and scatered arround non contiguous
memory blocks, I can use the following:

int *ary = new int[sizeX * sizeY]; // single memory block

Access x,y element as:

	 ary[y*sizeX + x]

Don't forget to do:  delete[] ary.
int Matrix based on the above might look like:

class Matrix {
	int* array;
	int m_width;
	public:
	Matrix( int w, int h ) : m_width( w ), array( new int[ w * h ] )
	{
	}

	~Matrix() {
		delete[] array;
	}
	int at( int x, int y ) const
	{
		return array[ index( x, y ) ];
	}
	protected:
	int index( int x, int y ) const { return x + m_width * y; }
};

This is certianly better memory layout but equaly certainly more
computation intensive.

We can now have lambdas return lvalue references, which can make things
a (much) more user friendly:

constexpr size_t num_cols = 0xF ;
constexpr size_t num_rows = 0xF ;
auto one_dim_array = new int[num_rows * num_cols];
auto two_mtx_elmnt = [num_cols, &one_dim_array](size_t row, size_t col) -> int&
{
   return one_dim_array[row*num_cols + col];
}
Use like :	 two_mtx_elmnt(i, j) = 5
Really just like modern macros. Could be the fastest solution.

A bit of a conceptual and practical mess, this is ..

Thus, from me, here is the all stack all static almost-a-pod variant.
In a standard WIDTH++ way.
*/

#pragma warning( push)
#pragma warning( disable: 4307 )

// NOTE: __COUNTER__ is also a GCC predefined macro 
// not just MSVC
// #define DBJ_UID  __COUNTER__ + dbj::util::dbj_get_seed()
#define DBJ_UID  __COUNTER__ + 100
// BIG WARNING: __COUNTER__ macro does not work in a loop. 
// It is a macro, a simple text substituton pre-processing mechanism
// that is: pre-processing text substitution only. Happens before compilation stages.

namespace dbj::arr {
	/*
	GPLv3 (c) 2018
	dbj's static matrix_type , almost a pod.
	*/
	template <
		typename	T,
		size_t		HEIGHT,
		size_t		WIDTH,
		size_t      UID_
	>
		class stack_matrix final
	{
	public:
		constexpr static inline size_t	MAX_WEIGHT = 0xFFFF; /* 65535 aka 64 kb*/
		using uid_type = size_t;

		// 'type' is like a 'this' here
		// without it code bellow will be much more complex to write and read
		using type = stack_matrix;

		/*	we also clean const and volatile if used "by accident" */
		using value_type = std::remove_cv_t<T>;
		using matrix_type = value_type[HEIGHT][WIDTH];
		using matrix_ref_type = value_type(&)[HEIGHT][WIDTH];
	private:
		/* here we enforce usage policies first, specifically

		The 64-bit PECOFF executable format used on Windows
		doesn't support creating executables that have
		a load size of greater than 2GB

		Since this matrix is all on stack we have choosen here
		much smaller MAX_WEIGHT then INT_MAX  which is 0x7FFFFFFF -- limits.h
		and thus enforce that as stack matrix size policy here
		*/
		static_assert(
			(HEIGHT * WIDTH * sizeof(value_type)) < type::MAX_WEIGHT,
			"Total weight of stack_matrix must not exceed 0xFFFF (65536) bytes"
			);

		static_assert(
			std::is_standard_layout_v <value_type>,
			"stack_matrix can be made out of standard layout types only"
			);

		/*
		almost just a pod 2D array on stack
		*/
		inline static value_type data_[HEIGHT][WIDTH]{};

		/*
		IMPORTANT: this is HEIGHT x WIDTH matrix
		*/

	public:
		/*
		we must implement some mechinsm so that
		every type definition does NOT share this same 2d native array
		this is the role of the UID__ template parameter

		please make sure you do understand how this makes for template definition

		typeof  stack_matrix<int,4,4,0> != typeof stack_matrix<int,4,4,1>

		thus: if not for the last parameter, all the stack matrices of the same size
		and type will share the same data.
		we add the last param and thus we have two different types, and keep them separate
		*/
		static constexpr const uid_type  uuid() { return UID_; }
		static constexpr char const * type_name() { return typeid(type).name(); }
		static constexpr size_t height() noexcept { return HEIGHT; };
		static constexpr size_t width() noexcept { return WIDTH; };
		static constexpr size_t weight() noexcept {

			static_assert(sizeof(matrix_type) == (HEIGHT * WIDTH * sizeof(value_type)));
			
			return sizeof(matrix_type);
			// same as: HEIGHT * WIDTH * sizeof(value_type);
			// same as sizeof(value_type[HEIGHT][WIDTH]) 
		};
		static constexpr size_t rank()		noexcept { return std::rank_v  <  matrix_type   >; }
		static constexpr size_t max_weight()	noexcept { return type::MAX_WEIGHT; };

		/*
		construcotr is largely irrelevant for anything but
		sanity checks of the implementation
		*/
		constexpr explicit stack_matrix()
		{
			static_assert(2 == std::rank_v  <  matrix_type   >);
			static_assert(HEIGHT == std::extent_v< matrix_type, 0 >);
			static_assert(WIDTH == std::extent_v< matrix_type, 1 >);
		}
		~stack_matrix() { }

		/*
		Not returning pointer but reference!
		this is criticaly important quality of this design
		alo this reference never becomes invalid as it refers to
		a static data block

		ALSO! this is the reason we have not implemented as 1D array
		we like to have the reference to the 2D NATIVE ARRAY available

		ALSO! this conforms to the best usage of satdnard WIDTH++
		to write the low level matrix routines.
		See the declaration (for example) of inner::multiply here
		That also plays well with C11 VLA's (in GCC only for now)
		*/
		constexpr  static matrix_ref_type data() noexcept
		{
			return matrix_ref_type(type::data_);
		}

		// data overload for changing the cells in the matrix
		// return type is a reference so we can both
		// set and get values in the matrix cells easily
		constexpr static value_type & data(size_t r, size_t c) noexcept
		{
			return (value_type &)(type::data_[r][c]);
		}

		// callback signature has to be
		// bool (*fun)( value_type & val, size_t row, size_t col);
		// if callback returns false, process stops
		// if callback throws the exception, process stops
		using callback = bool(*)(value_type &, size_t, size_t);

		/*
		for each row visit every column
		*/
		template<typename F>
		constexpr static void for_each(const F & fun)
		{
			for (size_t row_ = 0; row_ < HEIGHT; row_++) {
				for (size_t col_ = 0; col_ < WIDTH; col_++) {
					bool result =
						fun((value_type &)type::data_[row_][col_], row_, col_);
					if (result == false)
						return;
				}
			}
		}

		// utility function for printing to the console
		// F has to be a variadic print function
		// lambda example of such a function:
		//
		// inline auto print = 
		//     [](const auto & first_param, auto && ... params);
		//
		// note how F has to be able to print the type T of the matrix
		template<typename F>
		constexpr static void printarr(F print)
		{
			print("\n", type::type_name(), "\n");
			for (int r = 0; r < HEIGHT; r++)
			{
				print("\n{ ");
				for (int c = 0; c < WIDTH; c++)
					print(" {", type::data_[r][c], "} ");
				print(" }");
			}
			print("\n");
		}

		template<typename A, typename B, typename HEIGHT>
		friend void stack_matrix_multiply();

	}; // stack_matrix<T,HEIGHT,WIDTH,UID>

	namespace inner {

		/*
		C11 code to multiply two matrices:
		https://codereview.stackexchange.com/questions/179043/matrix-multiplication-using-functions-in-c?answertab=active#tab-top
		*/

		/*
		The definition of matrix multiplication is that:
		if C = A x B, for an n × m matrix A and an m × p matrix B, 
		then C dimension must be n × p 
		https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm

		Apparently the speed of this algporithm is: O(log n^3)
		And no one has made an matrix multiplication algorithm 
		faster than O(log n^2.37)
		If that makes a difference for you by no means use them
		fast algorithms.
		*/
		template<typename T, size_t N, size_t M, size_t P>
		inline void multiply(T(&a)[N][M], T(&b)[M][P], T(&c)[N][P])
		{
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < P; j++) {
					c[i][j] = 0;
					for (int k = 0; k < M; k++) {
						c[i][j] += a[i][k] * b[k][j];
					}
				}
			}
		}

		// one notch above the most simple iterative algorithm above
		// is the one bellow .. the recursive version
		// the only problem is it is very easily throwing a stack overflow
		// so it is not used and in here is just as my tumbstone devoted to
		// those developers who have falled searching for easy optimisations
		template<typename T, size_t N, size_t M, size_t P>
		inline void multi_rx(T(&a)[N][M], T(&b)[M][P], T(&c)[N][P])
		{
			// i and j are used to keep the current cell of 
			// result matrix WIDTH[i][j]. k is used to keep the
			// current column number of A[.][k] and row 
			// number of B[k][.] to be multiplied 
			static int i = 0, j = 0, k = 0;

			// If all height traversed. 
			if (i >= N)
				return;

			// If i < N 
			if (j < P)
			{
				if (k < M)
				{
					c[i][j] += a[i][k] * b[k][j];
					k++;
					multi_rx(a, b, c);
				}
				k = 0;	j++;
				multi_rx(a, b, c);
			}
			j = 0;	i++;
			multi_rx(a, b, c);
		}

		/*
		https://en.wikipedia.org/wiki/Freivalds%27_algorithm

		it seems this works on square matrices only?

		freivald(a,b,c) is "supposed" to return true if a x b == c
		see the article
		*/
		template<typename T, size_t n>
		inline bool freivald(T(&a)[n][n], T(&b)[n][n], T(&c)[n][n])
		{

			//random generation of the r vector containing only 0/1 as its elements
			double r[n][1];
			for (int i = 0; i < n; i++)
			{
				r[i][0] = rand() % 2;
			}

			//test A * (b*r) - (WIDTH*) = 0
			double br[n][1];
			for (int i = 0; i < n; i++)
			{
				for (int j = 0; j < 1; j++)
				{
					for (int k = 0; k < n; k++)
					{
						br[i][j] = br[i][j] + b[i][k] * r[k][j];
					}
				}
			}

			double cr[n][1];
			for (int i = 0; i < n; i++)
			{
				for (int j = 0; j < 1; j++)
				{
					for (int k = 0; k < n; k++)
					{
						cr[i][j] = cr[i][j] + c[i][k] * r[k][j];
					}
				}
			}
			double abr[n][1];
			for (int i = 0; i < n; i++)
			{
				for (int j = 0; j < 1; j++)
				{
					for (int k = 0; k < n; k++)
					{
						abr[i][j] = abr[i][j] + a[i][k] * br[k][j];
					}
				}
			}
			//    br = multiplyVector(b, r, n);
			//    cr = multiplyVector(c, r, n);
			//    abr = multiplyVector(a, br, n);

			//abr-cr
			for (int i = 0; i < n; i++)
			{
				abr[i][0] -= cr[i][0];
			}

			bool flag = true;
			for (int i = 0; i < n; i++)
			{
				if (abr[i][0] == 0)
					continue;
				else
					flag = false;
			}
			return flag;
		}
	} // inner
	/*
	  A[n][m] x B[m][p] = HEIGHT[n][p]

	stack_matrix<T,HEIGHT,WIDTH,UID> matrix is HEIGHT x WIDTH matrix
	*/
	template<typename A, typename B, typename HEIGHT >
	inline void stack_matrix_multiply()
	{
		// check types to match on all 3 matrices
		static_assert(std::is_same_v<A::value_type, B::value_type>,
			"\n\n" __FUNCSIG__ "\n");
		static_assert(std::is_same_v<A::value_type, HEIGHT::value_type>,
			"\n\n" __FUNCSIG__ "\n");
		// Rows of A must be the same as columns of B
		static_assert(A::width() == B::height(),
			"\n\n" __FUNCSIG__ "\n");
		// HEIGHT must be height of A x width of B
		static_assert(HEIGHT::height() == A::height(),
			"\n\n" __FUNCSIG__ "\n");
		static_assert(HEIGHT::width() == B::width(),
			"\n\n" __FUNCSIG__ "\n");

		/* template arguments are: < A::value_type, A::height(), A::width(), B::width() >*/
		::dbj::arr::inner::multiply(A::data_, B::data_, HEIGHT::data_);
	}

#pragma warning( pop )

} // eof namespace dbj::arr

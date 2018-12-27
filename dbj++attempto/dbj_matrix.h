#pragma once
#include "pch.h"
#include "dbj_uuid_hash.h"
/*
T[][N] would be called "array of array [N] of T" and be an incomplete matrix_type, 
while T[][] would be an invalid matrix_type (all except the last dimensions 
must have a known size). T[N][M] is "array [N] of array[M] of T", 
while T[sizeX] is "array [sizeX] of T" where T is a pointer to an int. 
Creating a dynamically 2d array works like this: 

 // C++11 onwards -- allocate (with initializatoin)
	auto array = new double[M][N]();
	delete [] array ;

It will create an array of an allocated-matrix_type int[X][Y]. 
This is a "hole" in C++'s matrix_type system, since the ordinary matrix_type system
of C++ doesn't have array dimensions with sizes not known at compile time, 
thus these are called "allocated types"

But. If you are worrited about above being slow and scatered arround non contiguous 
memory bloakcs, you can use the following:

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
a little more user friendly: 

constexpr size_t num_cols = 0xF ;
constexpr size_t num_rows = 0xF ;
auto one_dim_array = new int[num_rows * num_cols]; 
auto two_dim_array = [num_cols, &one_dim_array](size_t row, size_t col) -> int& 
{ 
   return one_dim_array[row*num_cols + col]; 
} 

Use like :

     two_dim_array(i, j) = 5
Really just like modern macros. Could be the fastest solution.

Whatever, first the all stack all static almost-a-pod variant.
In a standard C++ way.
*/

#pragma warning( push)
#pragma warning( disable: 4307 )

// NOTE: __COUNTER__ is also a GCC predefined macro 
// not just MSVC
#define DBJ_UID  __COUNTER__ + dbj::util::dbj_get_seed()
// BIG WARNING: __COUNTER__ macro does not work in a loop. 
// It is a macro, a simple text substituton pre-processing mechanism

/*
GPLv3 (c) 2018 
dbj's static matrix_type , almost a pod.
*/
template <
	typename	T,
	size_t		R,
	size_t		C,
	unsigned long UID_,
	size_t		MAXSIZE = 0xFFFF 
	/*
	INT_MAX as a absolute max is 0x7FFFFFFF -- limits.h
	but it is very unlikely your compiler will allow such 
	a large array on the stack to be created
	*/
>
class compile_time_stack_matrix final
{
public:
	// this is like a 'this' for static template instances
	// without it code bellow will be much more complex
	using type = compile_time_stack_matrix;
	/*	we also clean const and volatile if used "by accident" */
	using value_type		= std::remove_cv_t<T>;
	using matrix_type		= value_type[R][C];
	using matrix_ref_type	= value_type(&)[R][C];
private:
	/* here we enforce usage policies first, specifically

	The 64-bit PECOFF executable format used on Windows
	doesn't support creating executables that have
	a load size of greater than 2GB

	Since this is all on stack we have choosen here 
	much smaller MAXSIZE then INT_MAX  which is 0x7FFFFFFF -- limits.h
	and thus enforce thzt as stack matrix size policy here
	*/
	static_assert(
		(R * C * sizeof(value_type)) < /* = */ MAXSIZE,
		"Total size of compile_time_stack_matrix must not exceed 0xFFFF (65536) bytes"
		);

	static_assert(
		std::is_pod_v<value_type>,
		"compile_time_stack_matrix can be made out of POD types only"
		);

	/*	
	almost just a pod 2D array on stack 
	since it is static we must implement some mechinsm so that
	every definition does NOT share this same 2d native array
	this is the role of the UID__ template parameter
	*/
	inline static value_type data_[R][C]{};

public:
	/*
	please make sure you do understan how this makes for template definition 
	wide uid, because it is a template parameter.
	compile_time_stack_matrix<int,4,4,0>
	is different type vs
	compile_time_stack_matrix<int,4,4,1>
	*/
	static constexpr const unsigned long  uuid() { return UID_; }
	static constexpr const char * type_name() { return typeid(matrix_type).name(); }
	static constexpr size_t rows() noexcept { return R; };
	static constexpr size_t cols() noexcept { return C; };
	static constexpr size_t size() noexcept {
		return sizeof(matrix_type);
		// same as: R * C * sizeof(value_type);
		// same as sizeof(value_type[R][C]) 
	};
	static constexpr size_t rank()		noexcept { return std::rank_v  <  matrix_type   >; }
	static constexpr size_t max_size()	noexcept { return MAXSIZE; };

	/*
	construcotr is largely irrelevant for anything but
	sanity cheks of the implementation
	*/
	constexpr explicit compile_time_stack_matrix()
	{ 
		static_assert(2 == std::rank_v  <  matrix_type   >);
		static_assert(R == std::extent_v< matrix_type, 0 >);
		static_assert(C == std::extent_v< matrix_type, 1 >);
	}
	~compile_time_stack_matrix() { }

	/* 
	Not returning pointer but reference 
	this is criticaly important quality of this design
	alo this reference never becomes ivalid as it refers to 
	a static data block
	*/
	constexpr  static matrix_ref_type data() noexcept 
	{ return matrix_ref_type(type::data_); }

	// data overload for changing the cells in the matrix
	// return type is a reference so we can both
	// set and get values in the matrix cells easily
	constexpr static value_type & data (size_t r, size_t c) noexcept
	{ return (value_type &)(type::data_[r][c]); }

	// callback signature has to be
	// bool (*fun)( value_type & val, size_t row, size_t col);
	// if callback returns false, process stops
	// if callback throws the exception, process stops
	using callback =  void (*)( value_type & , size_t , size_t );

	/*
	for each row visit every column
	*/
	template<typename F>
	constexpr static void for_each(const F & fun)
	{
		for (size_t row_ = 0; row_ < R; row_++)	{
			for (size_t col_ = 0; col_ < C; col_++) {
				if ( false == fun(
					(value_type &)type::data_[row_][col_], row_, col_
				)) return ;
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
	template<typename F>
	constexpr static void printarr(F print)
	{
		print("\n", type::type_name());
		for (int r = 0; r < R; r++)
		{
			print("\n[ ");
			for (int c = 0; c < C; c++)
			// note how F has to be able to print type T of this matrix
				print(" [", type::data_[r][c], "] ");
			print(" ]");
		}
		print("\n");
	}

};

#if 1
template< typename MX, typename T >
void test_mx ( T new_val)
{
	dbj::console::print("\n ID: ", MX::uuid(),", size: ",MX::size(), ", rank: ", MX::rank());
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
MX test_mx_arg_retval(MX the_mx)
{
	// leave the trace
	the_mx.data(
		the_mx.rows() - size_t(1), 
		the_mx.cols() - size_t(1)
	) = typename MX::value_type(1234);
	return the_mx;
}

DBJ_TEST_UNIT(dbj_static_matrix) {

	constexpr size_t R = 3, C = 3;

	// dbj static matrix solution
	using mx9a = compile_time_stack_matrix<int, R, C, DBJ_UID >;
	using mx9b = compile_time_stack_matrix<int, R, C, DBJ_UID >;

	_ASSERTE(mx9a::uuid() != mx9b::uuid());

	test_mx<mx9a>(  0   );
	test_mx<mx9b>( 100  );

	// instances are ok but perfectly
	// redundant in this context
	mx9a mxa = test_mx_arg_retval(mx9a());
	mx9b mxb = test_mx_arg_retval(mx9b());

	mxa.printarr(dbj::console::print);
	mxb.printarr(dbj::console::print);
}

// example call
// printarr((int(&)[3][3])array);
template<typename T, size_t M, size_t N>
inline void printarr(const T(&array)[M][N])
{
	using dbj::console::print;
	print("\n", typeid(T(&)[M][N]).name());
	// 
	for (int r = 0; r < M; r++)
	{
		print("\n[ ");
		for (int c = 0; c < N; c++)
			print(" [", array[r][c], "] ");
		print(" ]");
	}
	print("\n");
}

DBJ_TEST_UNIT(cpp_dynamic_arrays) 
{
	constexpr size_t R = 3;
	constexpr size_t C = 3;
	// shared pointer based solution
	using matriq = std::shared_ptr<int[R][C]>;
	matriq m3(new int[R][C]{ {1,2,3},{1,2,3},{1,2,3} }) ;
	m3.get()[1][1] = 42 ;
	printarr((int(&)[R][C])(*m3.get()));

	// shared pointer based *faster* solution
	using matriq2 = std::shared_ptr<int[]>;
	matriq2 m32(new int[R * C]{ 1,2,3,1,2,3,1,2,3 });
	int r = 1, c = 1;
	// *(m32.get() + r * C + c) = 42;
	m32.get()[(r * C + c)] = 42;
	printarr((int(&)[R][C])(*m32.get()));
}

#endif

#pragma warning( pop )

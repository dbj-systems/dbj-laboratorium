#include "pch.h"
#include <assert.h>
#define DBJ_DB_TESTING
#include "..\dbj++sql\dbj++sql.h"

// using callback_type = int(*)(
// void * , int , char ** , char ** );
// NOTE! carefull with large data sets, 
// as this is called once per row
int dbj_sqlite_callback(
	void *  a_param[[maybe_unused]], 
	int argc, char **  argv, 
	char ** column
)
{
	using dbj::console::print;
	// print the row
	print("\n");
	// we need to know the structure of the row 
	for (int i = 0; i < argc; i++)
		print("\t", i , ": ", column[i], " --> [" , argv[i] , "]" );
	return 0;
}

/*
once per each row
*/
int dbj_sqlite_result_row_callback( 
	const size_t row_id ,
	/* this is giving us column count and column names */
	[[maybe_unused]] const std::vector<std::string> & col_names ,
	const dbj::sqlite::value_decoder & val_user 
)
{
	using dbj::console::print;
	// 'automagic' transform to std::string
	// of the column 0 value for this row
	std::string   word_ = val_user(0) ;
	print("\n\t", row_id, "\t", word_);

	// all these should provoke exception
	// TODO: but they don't -- currently
	long   DBJ_MAYBE( number_ ) = val_user(0) ;
	double DBJ_MAYBE( real_ ) = val_user(0) ;
	return SQLITE_OK;
}

DBJ_TEST_UNIT(dbj_sql_lite) 
{
	dbj::sqlite::test_insert();
	dbj::console::print("\ndbj_sqlite_callback\n");
	dbj::sqlite::test_select(dbj_sqlite_callback);
	dbj::console::print("\ndbj_sqlite_statement_user\n");
	dbj::sqlite::test_statement_using(dbj_sqlite_result_row_callback);
}

#undef DBJ_DB_TESTING

namespace bulk_free {

	/*
	NOTE: must place NULL as the last arg!
	      max args is 255
	*/
	void free_free_set_them_free(void * vp, ...)
	{
		size_t max_args = 255; size_t arg_count = 0; 
		va_list marker;
		va_start(marker, vp); /* Initialize variable arguments. */
		while (vp != NULL)
		{
			free(vp);
			vp = NULL;
			vp = va_arg(marker, void *);
			/* feeble attempt to make it safer  */
			if ( ++arg_count == max_args ) break ;
		}
		va_end(marker);   /* Reset variable argument list. */
	}

#define FREE(...) free_free_set_them_free((void *)__VA_ARGS__, NULL)

	DBJ_TEST_UNIT(bulk_free)
	{ 
		constexpr size_t newlen = 1024;
		constexpr size_t ARRSZ = 9;
		typedef char  * arr_type[ARRSZ];
		arr_type slave = {
	(char*)calloc( newlen, sizeof(char)), /* 0 */
	(char*)calloc( newlen, sizeof(char)),
	(char*)calloc( newlen, sizeof(char)),
	(char*)calloc( newlen, sizeof(char)),
	(char*)calloc( newlen, sizeof(char)),
	(char*)calloc( newlen, sizeof(char)),
	(char*)calloc( newlen, sizeof(char)),
	(char*)calloc( newlen, sizeof(char)),
	(char*)calloc( newlen, sizeof(char)) /* 8 */
		};

		arr_type * copy_ = (arr_type*)malloc( ARRSZ * newlen );

		for (size_t n = 0; n < ARRSZ; ++n) (*copy_)[n] = _strdup (slave[n]);

	FREE(
		slave[0], slave[1], slave[2], slave[3], slave[4], 
		slave[5], slave[6], slave[7], slave[8]
	);

	FREE(
		(*copy_)[0], (*copy_)[1], (*copy_)[2], (*copy_)[3], (*copy_)[4], 
		(*copy_)[5], (*copy_)[6], (*copy_)[7], (*copy_)[8]
	);

	char * p = (char*)malloc(42);
	free_free_set_them_free(p, NULL);

	}

}

using namespace ::std ;

namespace dbj {
	template<class T, T v>
	struct integral_constant 
	{
		static_assert( is_integral_v<T>);

		static constexpr T value = v;
		typedef T value_type;
		typedef integral_constant type;
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; }
	};

	template<int i>
	constexpr inline auto int_c = integral_constant<int, i>{};

	template <typename V, V v, typename U, U u>
	constexpr auto
		operator+(integral_constant<V, v>, integral_constant<U, u>)
	{
		return integral_constant<decltype(v + u), v + u>{};
	}
}

DBJ_TEST_UNIT(compile_time_entities_as_objects_instead_of_types)
{
	{
		// integral constant as type
		using one = dbj::integral_constant<int, 1>;
		// integral constant as object
		auto  one_rt = dbj::integral_constant<int, 1>{};
		// integral constant RT value
		constexpr int one_constexpr = decltype(one_rt)::value;
	}
	{
using zero_int_type = dbj::integral_constant<int, 0>;
	
auto succ = [] (auto N) {
	return dbj::integral_constant<int, decltype(N)::value + 1 >{};
};

		auto zero = dbj::int_c<0> ;

		auto three = dbj::int_c<1> +dbj::int_c<2>;

/*
		auto one = succ(zero);
		auto two = succ(one);
		auto three = succ(two);

		auto z_ = zero() ;
		auto o_ = one() ;
		auto t_ = two() ;
		auto h_ = three() ;
*/
		system("pause");
	}
}

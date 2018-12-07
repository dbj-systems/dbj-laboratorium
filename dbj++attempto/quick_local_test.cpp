#include "pch.h"
#include <assert.h>

namespace palindromes_research {
	extern "C" inline bool petar_pal(const char* str)
	{
		char* a = (char*)str,
			*p = a,
			*q = p;
		size_t n = strlen(str);
		for (p = a, q = a + n - 1;
			p < q;
			p++, q--
			)
		{
			if (*p != *q) return false;
		}
		return true;
	}


	extern "C" inline bool is_pal(const char* str) {
		char* s = (char*)str;
		char* e = (char*)str;
		// pomeri e na str kraj 
		// tj na '\0'
		while (*e) e++;
		// vrati ga na zadnje slovo 
		--e;
		// imamo dva setacha
		while (s < e) {
			// razliciti sadrzaji s'leva
			// i s'desna 
			// dakle ne moze biti palindrom
			if (*s != *e) return false;
			// levi napreduje u desno
			++s;
			// desni napreduje u levo
			--e;
			// *moguca* optimizacija je 
			// da je desni levo od levog
			if (e < s) break;
			// prekidamo petlju 
			// rezultat je true
		}
		return true;
	}


	inline void test_palindroma(const char * word_ = "012345678909876543210")
	{
		_ASSERTE(is_pal("ANA"));
		_ASSERTE(is_pal(word_));
		_ASSERTE(petar_pal("ANA"));
		_ASSERTE(petar_pal(word_));
	}
} // palindromes_research

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

		_ASSERTE( copy_ );

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
		system("pause");
*/
	}
}

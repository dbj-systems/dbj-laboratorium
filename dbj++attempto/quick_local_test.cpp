#include "pch.h"
#include <assert.h>
#include <memory>
#include <dbj++clib/dbjclib.h>
#include <functional>
#include <optional>
#include "dbj_ref_node.h"

using namespace ::std;

namespace dbj::samples::palindromes_research {
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

namespace dbj::samples::bulk_free {

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

		DBJ_MULTI_FREE(
		slave[0], slave[1], slave[2], slave[3], slave[4], 
		slave[5], slave[6], slave[7], slave[8]
	);

		DBJ_MULTI_FREE(
		(*copy_)[0], (*copy_)[1], (*copy_)[2], (*copy_)[3], (*copy_)[4], 
		(*copy_)[5], (*copy_)[6], (*copy_)[7], (*copy_)[8]
	);

	char * p = (char*)malloc(42);
	free_free_set_them_free(p, NULL);

	}

}


namespace dbj::samples {
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
	 DBJ_TEST_ATOM(DBJ_ARR_LEN("ABC") );

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
		auto zero = dbj::samples::int_c<0> ;
		auto three = dbj::samples::int_c<1> +dbj::samples::int_c<2>;
	}
}


namespace dbj::samples::init {

	inline constexpr unsigned iid(unsigned arg_) { return arg_; }

	// create and keep single instance of type T
	// IID is Instance ID used
	// otherwse one template definition for one T
	// will hold all the instances of the T
	// example :
	// singletor<int,1>::construct(42) 
	// singletor<int,1>::construct(24)
	// will create two different process wide int's
	// 42 and 24
	// the easy way to create different globals:
	// using global1 = singletor<int,__COUNTER__> ;
	// using global2 = singletor<int,__COUNTER__> ;
	// above are two different definitions
	template<
		typename T,
		int IID_ARG,
		std::enable_if_t< std::is_constructible_v<T>, int> = IID_ARG
	>
	struct singletor final
	{
		using value_type = T;

		constexpr int id() {
			int retval{ IID_ARG };
			return retval;
		}

		// T does not need to be movable 
		// construct the single_instance_ of T 
		// and keep it
		template< typename ...A>
		static T const & construct(A...args)
		{
			// if T is function bellow will not compile
			static T	single_instance_(args...);
			return		single_instance_;
		}
	};

} // init

namespace dbj::samples::inner {

	template<unsigned K>
	using ascii_ordinal = ::dbj::inside_t<unsigned, K, 0, 127>;

	using ascii_ordinal_t = ::dbj::core::util::insider<unsigned, 0U, 127U>;

	// creator functions must return values
	// this is to prevent them keeping static instances
	// this is actually what is also known
	// as 'creation policy'
	bool is_ascii(const char * arg_) 
	{ 
		::dbj::buf::yanb str(arg_);
		size_t N = str.size();
		for (size_t k = 0; k < N; k++) {
			ascii_ordinal_t  ord = int(arg_[k]);
			if (!ord.valid()) return false;
		}
		return true;
	};


	//-------------------------------------------------------
	DBJ_TEST_UNIT( test_the_initor )
	{
		using namespace init;
		using namespace ::dbj::util ;
		using      ::dbj::console::print;

		// this line actually creates the function definition
		// and declaration
		print("\n", once(is_ascii, "Only one"));
		// second call to prove we already made and are holding 
		// the single instance
		print("\n", once(is_ascii, "Not two"));

		print("\n", once( ::dbj::num::random_from_to<int>, 24, 42 ));

		// this declares + defines the type
		// + the type wide instance of std::string
		// in other words: process wide instance
		auto stringetor_1 = singletor<std::string, 42 >() ;
		auto stringetor_2 = singletor<std::string, 24 >() ;

		auto strinter = [&](auto strtor_ , auto arg_ ) {
			using      ::dbj::console::print;
			print("\nid:",
				// the first call wins
				// the first call defines the instance 
				// 'stringetor' is just an type alias
				strtor_.id(),
				"\tvalue:  ",
				strtor_.construct(arg_)
			);
		};

		strinter(stringetor_1, "A");
		strinter(stringetor_1, "B"); // it stays "A"
		strinter(stringetor_2, "C");
		strinter(stringetor_2, "D"); // it stays "C"
	}
}

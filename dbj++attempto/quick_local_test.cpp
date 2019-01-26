#include "pch.h"
#include <assert.h>
#include <memory>

using namespace ::std;

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


namespace init {

	inline constexpr unsigned iid(unsigned arg_) { return arg_; }

	// create and keep single instance of type T
	// IID is Instance ID used
	// otherwse one template definition for one T
	// will hold all the instances of the T
	// example of usage now:
	// singletor<int,1> 
	// singletor<int,2> 
	// are two different types
	// singletor<int,1>::call(42) 
	// singletor<int,1>::call(24)
	// will create two different process wide int's
	// 42 and 24
	// the easy way to create definitions
	// using global1 = singletor<int,__LINE__> ;
	// using global2 = singletor<int,__LINE__> ;
	// above are two different definitions
	template<typename T, int IID_ARG >
	struct singletor final
	{
		using value_type = T;

		static_assert(std::is_constructible_v<T>
			, "\n\nT must be constructible\n");

		constexpr int id() {
			int retval{ IID_ARG };
			return retval;
		}

		// T does not need to be movable 
		template< typename ...A>
		T const & call(A...args)
		{
			// if T is function bellow will not compile
			static T	single_instance_(args...);
			return		single_instance_;
		}
	};

	// function pointer version
	// notice how it defineds the signature to which 
	// creator function has to conform
	// also it is a function, not a type
	// it just returns the *value*
	// of the required type
	// made inside it
	template<typename RT, typename... ATs>
	inline RT const & once(RT (* creator )(ATs...), ATs ... args)
	{
		static_assert(! std::is_reference_v<RT>, 
			"\n\nStatic assert:\tcreator return type must *not* be a reference\n" );
		static_assert(! std::is_const_v<RT>    , 
			"\n\nStatic assert:\tcreator return type must *not* be const\n" );
		static_assert(  std::is_copy_constructible_v <RT>,
			"\n\nStatic assert:\tcreator return type must be copy constructible\n" );
		static_assert(  std::is_move_constructible_v<RT>,
			"\n\nStatic assert:\tcreator return type must be move constructible\n" );

		static RT singleton_{ creator(args...) };
		return singleton_;
	}

#ifndef _MSC_VER

	template <auto RT> struct starter;

	template<typename RT, typename... ATs, RT(*pF)(ATs...)>
	struct starter<pF>  final
	{
		static RT const & call(ATs ... args) {
			static RT const & singleton_ = pF(args...);
			return singleton_;
		}
	};

	template<typename RT, typename... ATs>
	inline RT const & once(RT(*pF)(ATs...), ATs ... args)
	{
		static RT const & singleton_ = pF(args...);
		return singleton_;
	}
	/*
	using namespace std;
	//--------------------------------------------
	std::string highlander(const char * arg) { return { arg }; };
	//--------------------------------------------
	int main()
	{
		using  make_highlander = starter<highlander>;

		cout << "Hello " << make_highlander::call("Highlander").data() << endl;
		cout << "Hello " << make_highlander::call("Dumbleby ?").data() << endl;
		cout << "Hello " << make_highlander::call("Michele  ?").data() << endl;

		cout << "Hello " << once(highlander, "Again!").data() << endl;
		cout << "Hello " << once(highlander, "NOT").data() << endl;
	}
	*/
#endif // !_MSC_VER

} // init

namespace {

	// creator functions must return values
	// this is to prevent them keeping static instances
	// this is actually what is also known
	// as 'creation policy'
	::std::string highlander(const char * arg_) { 
		return ::std::string(arg_);
	};

	int randomizer( int min, int max) 
	{
		std::random_device seed;
		std::mt19937 rng(seed());
		std::uniform_int_distribution<int> gen(min, max); // uniform, unbiased
		int rand_val = gen(rng);
		return rand_val;
	};

	//-------------------------------------------------------

	//----------------------------------------------------------------
	DBJ_TEST_UNIT( test_the_initor )
	{
		using namespace init;
		using      ::dbj::console::print;

		// this line actually creates the function definition
		// and declaration
		print("\n", once(highlander, "Only one"));
		// second call to prove we already made and are holding 
		// the single instance
		print("\n", once(highlander, "Not two"));

		print("\n", once(randomizer, 24, 42 ));

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
				// 'stringetor' is just an type alis
				strtor_.id(),
				"\tvalue:  ",
				strtor_.call(arg_)
			);
		};

		strinter(stringetor_1, "A");
		strinter(stringetor_1, "B");
		strinter(stringetor_2, "C");
		strinter(stringetor_2, "D");
	}
}

#include "pch.h"
//#include <assert.h>
//#include <memory>
#include <string_view>
#include <dbj++clib/dbjclib.h>
//#include <functional>
//#include <optional>
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


	DBJ_TEST_UNIT( array_uniq_ptr )
	{
		using ::dbj::console::print;
		auto mover = [](auto arg_) { return arg_ ;  };
		auto  [ arr_size, smart_p ] = dbj::make_smart_array_pair<char>(BUFSIZ);

		unique_ptr_buffer<char>		buffy = "Buffy"sv ;
		unique_ptr_buffer<wchar_t>	duffy(L"Duffy"sv);

		print("\n\nbuffy: ", buffy); print(L"\nduffy: ", duffy);

		auto buffy_copy =  buffy ;
		auto duffy_copy = mover( duffy ) ;

		print("\n\nbuffy copy: ", buffy_copy, "\nduffy copy: ", duffy_copy);
	}
	// creator functions must return values
	// this is to prevent them keeping static instances
	// this is actually what is also known
	// as 'creation policy'
	bool is_ascii(const char * arg_) 
	{ 
		using ascii_ordinal_t = ::dbj::core::util::insider<unsigned, 0U, 127U>;

		const size_t N =  strlen( arg_ ) ;
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
#pragma region unique_ptr based buffer

	using up_buffer = dbj::unique_ptr<char[]>;
	using up_buffer_w = dbj::unique_ptr<wchar_t[]>;

	template< class T  >
	inline constexpr size_t up_buffer_size(std::unique_ptr<T[]> const& source)
	{
		using UP = std::unique_ptr<T[]>;
		UP::element_type* ep_ = source.get();
		return (sizeof(ep_) / sizeof(ep_[0]));
	}

	template< class T  >
	inline std::unique_ptr<T[]> up_buffer_copy(std::unique_ptr<T[]> const & source)
	{
		using UP = std::unique_ptr<T[]>;
		UP::element_type * ep_ = source.get();
		const auto sze_ = ( sizeof(ep_) / sizeof(ep_[0]) );
		UP up_ = std::make_unique<UP::element_type[]>(sze_) ;
		std::copy(source.get(), source.get() + sze_, up_.get());
		return up_;
	}

	template< class char_type,
		typename UP = std::unique_ptr<char_type[]> >
		inline constexpr UP up_buffer_make ( size_t const & sze_ )
	{
		UP up = std::make_unique<char_type[]>(sze_ + 1) ;
		up[sze_] = char_type(0);
		return up;
	}

	template< class char_type , 
		typename UP = std::unique_ptr<char_type[]> ,
		typename SV = std::basic_string_view<char_type>
	>
	inline constexpr UP up_buffer_make ( char_type const * sliteral_ )
	{
		SV sview_{  sliteral_  };
		UP up = up_buffer_make<char_type>( sview_.size() );

		std::copy(sview_.begin(), sview_.end(), up.get());
		// add the string terminator
		up[sview_.size()] = char_type(0);
		return up;
	}	
	
	template< class char_type , 
		typename UP = std::unique_ptr<char_type[]> ,
		typename SV = std::basic_string_view<char_type>
	>
	inline constexpr UP up_buffer_make ( char_type const * sliteral_, size_t length_ )
	{
		UP up = up_buffer_make<char_type>(length_ + 1);
			std::copy(sliteral_, sliteral_ + length_ , up.get());
		// add the string terminator
		up[length_] = char_type(0);
		return up;
	}

	template< class CHAR, size_t N >
	inline constexpr
		typename std::array< typename std::remove_cv_t<CHAR>, N >
		sliteral_to_std_char_array
		(CHAR(&sliteral_)[N])
	{
		return arr::native_to_std_array(sliteral_);
	}

	inline  std::unique_ptr<char[]> 
		operator""_buffer( const char* sliteral_, size_t length_)
	{
		return up_buffer_make( sliteral_ , length_);
	}

	inline  std::unique_ptr<wchar_t[]> 
		operator""_buffer( const wchar_t* sliteral_, size_t length_)
	{
		return up_buffer_make( sliteral_ , length_);
	}

	template< char ... Chs >
	inline constexpr decltype(auto) operator"" _std_char_array( )
	{
		return  std::array { Chs... } ;
	}


	DBJ_TEST_UNIT(unique_ptr_simple_copy)
	{
		using namespace std;

		auto std_arr = sliteral_to_std_char_array("WOW?!");

		// std::array{ '1','2','3' }
		constexpr auto a_1 = 123_std_char_array;

		// std::array{ '0','x','1','2' }
		constexpr auto a_2 = 0x12_std_char_array;

		// std::array{ '4'.'2','.','1','3' }
		constexpr auto a_3 = 42.13_std_char_array;

		up_buffer wonder = "STRING LITERAL"_buffer;
		auto wonder_wall = L"WIDE STRING LITERAL"_buffer;
		up_buffer buffy{ up_buffer_make("Buffy") };
		auto buffy_size = up_buffer_size( buffy );
		auto buffy_count = _countof("Buffy");

		auto up = up_buffer_copy( buffy );

	console::print("\n\n", up);
	}
#pragma endregion unique_ptr copy
}

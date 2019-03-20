#pragma once
// requires "dbj_array.h"
#include <ctime>


/// <summary>
/// for more stringent test we 
/// test from outside of the
/// namespace that we are testing
/// </summary>
DBJ_TEST_SPACE_OPEN(dbj_array)

struct Color final  //this struct can't be modified
{
	double grey;
	double rgb[3];
};

#define make_color( x, a, b ) Color x { a, b[0], b[1], b[2] }

DBJ_TEST_UNIT( templated_memcpy ) {

	double myRGB[3]{ 2, 6, 9 };

	make_color( c, 10, myRGB ) ; // This line doesn't work

	dbj::arr::array_copy( c.rgb, myRGB );
}

/*------------------------------------------------------*/

inline auto default_element_output_ =
[&](size_t j, const auto & element,
	const char * fmt_str = " %zd:%d ")
{
	printf(fmt_str, j++, element);
};

/// <summary>
/// default arf print function
/// </summary>
template< typename T, size_t N, typename FUN >
inline auto default_print(
	const dbj::narf::wrapper<T, N> & arf,
	FUN outfun_,
	const char * prefix = "\nArray",
	const char * suffix = ""
) {
	printf("%s { ", prefix);
	dbj::narf::apply(arf, outfun_);
	printf("} %s", suffix);
}



/// <summary>
/// some generic utility function 
/// with argument declared as 
/// native array reference
/// </summary>
template<typename T, std::size_t N >
inline void native_arr_consumer(T(&arr_ref)[N]) {
	for (auto & element : arr_ref) {
		__noop;
	}
}

inline decltype(auto)
few_creation_examples() {

	int iarr[]{ 0,1,2,3,4,5,6,7,8,9 };

	auto narf_0 = DBJ_TEST_ATOM(
		dbj::narf::make(iarr)
	);

	/* this throws an segv on read
	static decltype(auto) narf_1 = DBJ_TEST_ATOM(
		dbj::narf::make({ "native","array","of", "narrow","string","literals" }));
	*/

	const char * narrs[]{ "native", "array", "of", "narrow", "string", "literals" };
	static auto narf_1 = DBJ_TEST_ATOM(dbj::narf::make(narrs));


	char const_char_array[]{ "native char array" };

	auto narf_2 = DBJ_TEST_ATOM(
		dbj::narf::make(const_char_array));

	return narf_1;
}

// this is how we get the reference
// to the contained native array
template< typename T>
inline auto different_ways_to_obtain_reference(T arf)
{
	decltype(auto) not_elegant = dbj::narf::data(arf);
	auto & standard = dbj::narf::data(arf);
	// change the native-arr contents
	// watch the retval in debugger to check 
	// the survival
	dbj::narf::apply(arf, [](auto idx, auto element) {
		char word[]{ (char)dbj::util::random(90, 64) };
		element = word;
	});
	// return by value
	return arf;
}

template< typename T>
inline auto calling_native_array_consumer
(T arf)
{
	// example of calling a function 
	// that requires
	// native array as argument
	native_arr_consumer(arf.get());
	// same as
	native_arr_consumer(dbj::narf::data(arf));
	// same as
	// using value_type = typename narf_type::type;
	// native_arr_consumer(arf);

	return arf;
}

DBJ_TEST_UNIT(native_dbj_array_handler)
{
	auto arf_0 = DBJ_TEST_ATOM(few_creation_examples());
	auto arf_1 = DBJ_TEST_ATOM(different_ways_to_obtain_reference(
		arf_0
	));
	auto arf_2 = DBJ_TEST_ATOM(calling_native_array_consumer(arf_1));
	// at this point all the arf's 
	// are referencing the same native array
	auto[bg, ed] = (dbj::narf::range(arf_0));

	// default get() is the pointer
	// due to array type decay 
	auto narrptr = arf_2.get();

	auto sze = DBJ_TEST_ATOM(dbj::narf::size(arf_2));
}

DBJ_TEST_UNIT(dbj_array_handler_ARH)
{
	{
		// the "C" way
		char arr_of_chars[]{ 'A','B','C' };
		char(&ref_to_arr_of_chars)[3] = arr_of_chars;
	}
	{   // manual C++ old school
		std::array<char, 3> three_chars{ 'A','B','C' };
		const char(&uar)[3] =
			(*(char(*)[3])three_chars.data());
	}
	// the modern C++ dbj way
	using A16 = dbj::arr::ARH<int, 3>;
	A16::ARR arr{ 1,2,3 };
	A16::ARP arp = DBJ_TEST_ATOM(A16::to_arp(arr));
	A16::ARF arf = A16::to_arf(arr);

	// prove that the type is right
	auto DBJ_MAYBE(rdr0) =
		DBJ_TEST_ATOM(A16::to_vector(arf));

	/*
	testing the internal_array_reference

	decltype(auto) bellow reveals the underlying type
	namely it transform int* to int(&)[]
	that is reference to c array inside std::array
	*/
	A16::ARF arf2 = A16::to_arf(arr);
	A16::ARV rdr1[[maybe_unused]] = DBJ_TEST_ATOM(A16::to_vector(arf2));

}

DBJ_TEST_UNIT(a_bit_more_arh_narf_dancing) {


	// narf to temporary no can do 
	// --> auto narf_ = dbj::narf::make({ "A B C NARF" });
	// so
	char charr[]{ "A B C NARF" };
	auto narf_ = DBJ_TEST_ATOM(dbj::narf::make(charr));
	decltype(auto) narf_arf_ = DBJ_TEST_ATOM(dbj::narf::data(narf_));

	using CARH = dbj::arr::ARH<char, 255>;

	//CARH::ARR narf_to_arh
	//	= CARH::to_std_array( narf_arf_ );

	CARH::ARR std_arr{ "CHAR ARR" };

	CARH::ARF native_arr_reference
		= CARH::to_arf(std_arr);

	CARH::ARR std_aray
		= DBJ_TEST_ATOM(CARH::to_std_array(native_arr_reference));

	CARH::ARF literal_to_native_arr_reference
		= CARH::to_arf(std_arr);

}

#pragma region dbj_a_bit_more_native_array_fascination

template<typename T, size_t N>
using ref_to_arr = std::reference_wrapper<const T[N]>;

/*
the point we are showing here: c++ smart pointer
do handle array but do not treat them as array  references internaly
if smart ptr declared to hold int[],element type is int
if declared to hold int, element type is int again
*/
template<typename T, size_t N>
using ptr_to_arr = std::shared_ptr<T[N]>;

DBJ_TEST_UNIT(dbj_a_bit_more_native_array_fascination)
{
	// native array on the stack
// the point of creation
	int int3[]{ 1,2,3 };
	// creation of the reference object
	// to the native stack created array
	ref_to_arr<int, 3> i3 = std::cref(int3);
	// get to the reference of the native array contained
	const ref_to_arr<int, 3>::type & i3ref = i3.get();

	auto DBJ_MAYBE(well_is_it) = dbj::tt::is_array_(i3ref);

	// smart pointer is not a solution, why?
	ptr_to_arr<int, 3> DBJ_MAYBE(spi3) (new int[3]{ 1,2,3 });
	// OK
	_ASSERTE(2 == spi3[1]);

	// if declared to hold int[]
	// element type is int
	// if declared to hold int
	// element type is int
	ptr_to_arr<int, 3>::element_type * arr_p = spi3.get();

	// but we 'know' it is an array, so ...
	bool DBJ_MAYBE(is_it_then) = dbj::tt::is_array_((int(&)[3])(arr_p));
}
#pragma endregion

DBJ_TEST_UNIT(would_you_believe_it_more_native_array_utils)
{

	using C3 = ::dbj::arr::ARH<char, 3>;

	C3::ARV v_of_c{ 'A','B','C' };

	C3::ARR  DBJ_MAYBE(std_arr) = C3::from_vector(v_of_c);

	{  // manual way
		constexpr size_t N = 3;
		auto DBJ_MAYBE(std_arr_) = v_2_a<char, N>(v_of_c);
	}

	const char(&DBJ_MAYBE(ref_to_arr_of_chars))[3] =
		*(char(*)[3])v_of_c.data();

	const char(&DBJ_MAYBE(ref_to_arr_of_chars_2))[] =
		*(char(*)[])v_of_c.data();

	const size_t N = v_of_c.size();
	char * vla_ = (char*)::std::calloc(N, sizeof(char));

	std::copy(v_of_c.data(), v_of_c.data() + v_of_c.size(), vla_);

	auto DBJ_MAYBE(isit) = std::is_array_v< decltype(vla_)>;

	::std::free(vla_);

	auto DBJ_MAYBE(dummy) = true;
}
DBJ_TEST_SPACE_CLOSE

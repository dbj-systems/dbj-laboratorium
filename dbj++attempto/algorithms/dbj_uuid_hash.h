#pragma once
#include "../pch.h"

/*
Using WIN32/RPC create the UUID and UUID string representation
Then create HASH from that 

djb2 :: http://www.cse.yorku.ca/~oz/hash.html

this algorithm (k=33) was first reported by dan bernstein many years ago in comp.lang.c. 
another version of this algorithm (now favored by bernstein) uses xor: 
hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33 
(why it works better than many other constants, prime or not) has 
never been adequately explained.

	unsigned long hash(unsigned char *str)
	{
		unsigned long hash = 5381;
		int c;
		while (c = *str++)
			hash = ((hash << 5) + hash) + c; // hash * 33 + c 
		return hash;
	}
*/

#pragma warning( push)
#pragma warning( disable: 4307 )

DBJ_TEST_UNIT(dbj_uuid_hash) 
{
	constexpr std::uint64_t seed = dbj::util::dbj_get_seed();
	constexpr unsigned long hcode = dbj::util::hash(__FILE__);
	dbj::util::uuid && uuid_ = dbj::util::uuid() ;
	const char * DBJ_MAYBE(us_) = DBJ_TEST_ATOM( (const char *)uuid_.uuid_string() );
	long long    DBJ_MAYBE(hc_) = DBJ_TEST_ATOM( uuid_.hash_code()    );
}

template<typename T, unsigned long ID = __COUNTER__ >
struct K final {
	typedef K type;
	constexpr inline static const unsigned long id{ ID };
};

#define STR(x) #x
DBJ_TEST_UNIT(dbj_compile_time_random) 
{
//	unsigned long DBJ_MAYBE( drk ){ dbj_random_kiss() };

	constexpr auto isit = dbj::util::compile_time_random();

	auto test = [&](auto arg) {
		using T = decltype(arg);
		using K1 = K<T, dbj::util::hash(STR(K<T>)) >;
		using K2 = K<T, dbj::util::compile_time_random() >;
		dbj::console::print(
			"\n compile time tagged types\n ", typeid(K1).name(),
			"\n ", typeid(K2).name() 
		);
	};
	for (int j = 0; j < 3; ++j)
	{
		test(13);
	}
}
#undef STR
#pragma warning( pop )

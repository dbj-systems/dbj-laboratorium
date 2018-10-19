#pragma once
#include "pch.h"
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

// For UUID
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

namespace dbj::util {

	constexpr int dbj_atoi(const char *str)
	{
		int res = 0; // Initialize result 
		// Iterate through all characters of input string and 
		// update result 
		for (int i = 0; str[i] != '\0'; ++i)
			res = res * 10 + str[i] - '0';
		// return result. 
		return res;
	}

constexpr unsigned long dbj_get_seed()
{
	int hour = dbj_atoi(__TIME__);
	int min = dbj_atoi(__TIME__ + 3);
	int sec = dbj_atoi(__TIME__ + 6);
	return 10000 * hour + 100 * min + sec;
}

// as an example, one can call bellow like this
// constexpr inline unsigned long hash_code =
//	dbj::util::hash(__FILE__);
constexpr inline unsigned long hash(const char *str)
{
	unsigned long hash = 5381;
	int c = 0;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	return hash;
}

// for some reason MSVC does not allow for 
// this cast inside constexpr functions?
// GCC does allow
template<typename T, size_t N>
constexpr inline unsigned long hashit(const T(&str)[N]) {
	return dbj::util::hash(str);
}

class uuid final {
		mutable UUID uuid_{};
		const char * uuid_string_ = NULL;
		mutable unsigned long hash_{ 0 };
	public :
		explicit uuid() noexcept {
			RPC_STATUS ret_val = ::UuidCreate(&this->uuid_);
			_ASSERTE(ret_val == RPC_S_OK);
		}
		~uuid() {
			if (uuid_string_ != NULL) {
				::RpcStringFreeA((RPC_CSTR*)&uuid_string_);
				uuid_string_ = NULL;
			}
		}

		constexpr  const UUID & get () const noexcept {
			return this->uuid_;
		}

		constexpr const char * uuid_string() const noexcept {
			if (uuid_string_ == NULL) {
				::UuidToStringA(
					&this->get(), (RPC_CSTR*)&uuid_string_
				 );
				_ASSERTE(uuid_string_ != NULL);
			}
			return uuid_string_;
		}

		constexpr unsigned long hash_code() const noexcept {
			if (this->hash_ == 0) {
				// compute hash from uuid_string_
				this->hash_ = dbj::util::hash( this->uuid_string() );
			}
			return this->hash_;
		}
	};
} // dbj::util

DBJ_TEST_UNIT(dbj_uuid_hash) 
{
	constexpr unsigned long seed = dbj::util::dbj_get_seed();
	constexpr unsigned long hcode = dbj::util::hash(__FILE__);
	dbj::util::uuid && uuid_ = dbj::util::uuid() ;
	const char * DBJ_MAYBE(us_) = DBJ_TEST_ATOM( (const char *)uuid_.uuid_string() );
	long long    DBJ_MAYBE(hc_) = DBJ_TEST_ATOM( uuid_.hash_code()    );
}

namespace dbj_compile_time_random 
{
	constexpr const char timestamp[]{ __TIMESTAMP__ };
	constexpr auto timestamp_hash_{ dbj::util::dbj_atoi(__TIMESTAMP__) };
	
// inspired by 
// https://stackoverflow.com/questions/5355317/generate-a-random-number-using-the-c-preprocessor/17420032#17420032

constexpr inline long compile_time_random() {
	/*  Global static variables:
	(the seed changes on every minute) */
	auto z = 362436069 * timestamp_hash_;
	auto w = 521288629 * timestamp_hash_,
		jsr = 123456789 * timestamp_hash_,
		jcong = 380116160 * timestamp_hash_;

	z = (36969 * (z & 65535) + (z >> 16)) << 16;
	w = 18000 * (w & 65535) + (w >> 16) & 65535;
	auto mwc = (z + w);
	jsr = (jsr = (jsr = jsr ^ (jsr << 17)) ^ (jsr >> 13)) ^ (jsr << 5);
	auto shr3 = jsr;
	jcong = 69069 * jcong + 1234567;
	auto cong = jcong;
	auto kiss = (mwc^cong) + shr3;
	return (kiss < 0 ? -1 * kiss : kiss );
}


template<typename T, long long ID>
struct K {
	using type = K;
	constexpr inline static const long long id = ID;
};

#define STR(x) #x
	template<typename T>
	inline auto make_uniq_k() {
		// cool but. this makes one ID per type
		// as the ID is computed from type id string
		// we neen many ID's per type
		return K<T, dbj::util::hash(STR(K<T>)) >();
		return K<T, compile_time_random() >();
	}

}

DBJ_TEST_UNIT(dbj_compile_time_random) 
{
	using namespace dbj_compile_time_random;

	unsigned long DBJ_MAYBE( drk ){ dbj_random_kiss() };

	constexpr auto isit = compile_time_random();

	auto test = [&](auto arg) {
		using T = decltype(arg);
		using K1 = K<T, dbj::util::hash(STR(K<T>)) >;
		using K2 = K<T, compile_time_random() >;
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

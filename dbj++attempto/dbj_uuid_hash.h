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

// For UUID
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

namespace dbj::util {

	namespace inner {
		constexpr unsigned long hash(const char *str)
		{
			unsigned long hash = 5381;
			int c = 0;
			while ((c = *str++))
				hash = ((hash << 5) + hash) + c; // hash * 33 + c 
			return hash;
		}
	}

	inline const unsigned long uuid_hash( )
	{
		// Create a new uuid
		UUID uuid{};
		RPC_STATUS ret_val = ::UuidCreate(&uuid);
		_ASSERTE(ret_val == RPC_S_OK);
			// convert UUID to LPCSTR
			unsigned char * uuid_string_ = NULL;
			::UuidToStringA(&uuid, (RPC_CSTR*)&uuid_string_);
			_ASSERTE(uuid_string_ != NULL);
				// compute hash from uuid_string_
			unsigned long retval = inner::hash((const char *)uuid_string_);
				// free up the allocated string
				::RpcStringFreeA((RPC_CSTR*)&uuid_string_);
				uuid_string_ = NULL;
		return retval;
	}
} // dbj::util

DBJ_TEST_UNIT(dbj_uuid_hash) {
	DBJ_TEST_ATOM( dbj::util::uuid_hash() );
	DBJ_TEST_ATOM( dbj::util::uuid_hash() );
	DBJ_TEST_ATOM( dbj::util::uuid_hash() );
	DBJ_TEST_ATOM( dbj::util::uuid_hash() );
}

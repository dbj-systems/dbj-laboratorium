#pragma once

#pragma warning( push)
#pragma warning( disable: 4307 )

namespace dbj::util {

	extern "C" {
		constexpr std::uint64_t dbj_atoi( char const * str)
		{
			std::uint64_t res = 0; // Initialize result 
			// Iterate through all characters of input string and 
			// update result 
			for (std::uint64_t i = 0; str[i] != '\0'; ++i)
				res = res * 10 + str[i] - '0';
			// return result. 
			return res;
		}

		constexpr std::uint64_t dbj_get_seed()
		{
			std::uint64_t hour = dbj_atoi(__TIME__);
			std::uint64_t min = dbj_atoi(__TIME__ + 3);
			std::uint64_t sec = dbj_atoi(__TIME__ + 6);
			return 10000 * hour + 100 * min + sec;
		}

#pragma warning( push )  
#pragma warning( disable : 4706 ) 
		// as an example, one can call bellow like this
		// constexpr inline unsigned long hash_code =
		//	dbj::util::hash(__FILE__);
		constexpr inline unsigned long hash(const char *str)
		{
			// http://www.cse.yorku.ca/~oz/hash.html
			unsigned long hash = 5381;
			int c = 0;
			while ((c = *str++))
				hash = ((hash << 5) + hash) + c; // hash * 33 + c 
			return hash;
		}
#pragma warning( pop )  

	} // extern "C" linkage
	
	// native string to hash
	template< size_t N>
	constexpr inline unsigned long arr_to_hash (const char(&str)[N]) {
		return dbj::util::hash(str);
	}

	// native array to hash
	template<typename T, size_t N>
	constexpr inline unsigned long arr_to_hash(const T(&str)[N]) {
		return dbj::util::hash(str);
	}



	namespace inner
	{
		constexpr const char timestamp[]{ __TIMESTAMP__ };
		constexpr auto timestamp_hash_{ dbj::util::dbj_atoi(__TIMESTAMP__) };
	}
		// inspired by 
		// https://stackoverflow.com/questions/5355317/generate-a-random-number-using-the-c-preprocessor/17420032#17420032

	/*
	This is QUAZY RANDOM + it is the same on every call
	*/
		constexpr inline long compile_time_random() 
		{
			auto z = 362436069 * inner::timestamp_hash_;
			auto w = 521288629 * inner::timestamp_hash_,
				jsr = 123456789 * inner::timestamp_hash_,
				jcong = 380116160 * inner::timestamp_hash_;

			z = (36969 * (z & 65535) + (z >> 16)) << 16;
			w = 18000 * (w & 65535) + (w >> 16) & 65535;
			auto mwc = (z + w);
			jsr = (jsr = (jsr = jsr ^ (jsr << 17)) ^ (jsr >> 13)) ^ (jsr << 5);
			auto shr3 = jsr;
			jcong = 69069 * jcong + 1234567;
			auto cong = jcong;
			auto kiss = (mwc^cong) + shr3;
			return (kiss < 0 ? -1 * kiss : kiss);
		}

} //dbj::util

#pragma warning( pop )

/* inclusion of this file defines the kind of a licence used */
#include "../dbj_gpl_license.h"
/*
Copyright 2017 by dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

# if !defined(__STDC_VERSION__) ||  (__STDC_VERSION__ < 199901L)
/* Your compiler is not conforming to C99, since
   this requires the macro __STDC_VERSION__ to be set to the
   indicated value (or larger).

   NOTE: For C11, __STDC_VERSION__ == 201112L
*/
#error "Need a C99 compliant compiler, or better"
#endif

#define _CRT_SECURE_NO_WARNINGS
// we do the above since we use MSVC UCRT *from* this clang c code lib

#include "dbjclib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// in corecrt.h
// #define _STATIC_ASSERT(expr) typedef char __static_assert_t[(expr) != 0]
// _STATIC_ASSERT(printf("Ooops?"));

static const unsigned location_descriptor_cache_size	  
    = location_descriptor_file_name_size;

static location_descriptor
location_descriptor_cache[location_descriptor_cache_size];

// if register slot is false, the slot is free and if true the slot is used
#define CACHE_REGISTER_FREE_MARK false
#define CACHE_REGISTER_ACTV_MARK true
static bool cache_register[location_descriptor_cache_size] ;

static unsigned cache_register_in_overflow_state = false ;
static const unsigned cache_register_invalid_slot = (unsigned)-1;
static const unsigned cache_register_last_slot = location_descriptor_file_name_size - 1;
static const unsigned cache_register_first_slot = 0 ;


static unsigned cache_register_first_free_slot () {
	
	if ( cache_register_in_overflow_state) return cache_register_invalid_slot ;
	
	// search for the first free slot
	for (unsigned cache_register_walker = cache_register_first_slot;
		cache_register_walker != cache_register_last_slot;
		cache_register_walker ++)
	{
		// found it
		if (cache_register[cache_register_walker] == CACHE_REGISTER_FREE_MARK)
		{
			// mark it as active
			cache_register[cache_register_walker] = CACHE_REGISTER_ACTV_MARK;
			return cache_register_walker;
		}
	}
		// if here we have new overflow situation
		cache_register_in_overflow_state = true;
		return cache_register_invalid_slot;
}

static void cache_register_release_slot( unsigned registry_slot_index )
{
	cache_register[registry_slot_index] = CACHE_REGISTER_FREE_MARK; 
	// if here we can not have overflow situation
	cache_register_in_overflow_state = false;

};

// the implementation -------------------------------------------------

// return NULL if cache overflow
static struct location_descriptor *
	create_location_descriptor( const int line_, const char * file_ )
	{
		unsigned free_slot = cache_register_first_free_slot();

		if (free_slot == cache_register_invalid_slot) {
			return NULL;
		}
		location_descriptor * desc_ 
			= & location_descriptor_cache[free_slot];
				desc_->cache_index = free_slot;
				desc_->line = line_;
		// file_ length can be above location_descriptor_file_name_size
		// so this might clip the result
		strncpy(desc_->file, file_, location_descriptor_file_name_size );

		return desc_ ;
	}


static location_descriptor *  release_location_descriptor(location_descriptor ** locdesc_ )
{
	// make the empty descriptor, once
	static location_descriptor 
		empty_location_descriptor =	{ 0,0 };

	assert( NULL != *locdesc_);

	cache_register_release_slot( (**locdesc_).cache_index );
		(**locdesc_) = empty_location_descriptor;

	return (*locdesc_) = NULL;
}

// the interface construction -------------------------------------------------
LOCATION location_ = { create_location_descriptor, release_location_descriptor };

// EOF

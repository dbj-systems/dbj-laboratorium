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

#include "dbjclib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const unsigned location_descriptor_cache_size	  
    = error_descriptor_buffer_size;

static error_descriptor
error_descriptor_cache[location_descriptor_cache_size];

// if register slot is false, the slot is free and if true the slot is used
#define CACHE_REGISTER_FREE_MARK false
#define CACHE_REGISTER_ACTV_MARK true
static bool cache_register[location_descriptor_cache_size] ;

static unsigned cache_register_in_overflow_state = false ;
static const unsigned cache_register_invalid_slot = (unsigned)-1;
static const unsigned cache_register_last_slot = error_descriptor_buffer_size - 1;
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

static const char * unknown_error_location = "Unknown error location";
static const char * unknown_error_message  = "Unknown error message";

// return NULL if cache overflow
static struct error_descriptor *
	create_error_descriptor( 
		const int line_, const char * file_ ,
		const int code_, const char * msg_ 
	)
	{
	if (file_ == NULL) file_ = unknown_error_location;
	if (msg_ == NULL)  msg_  = unknown_error_message;

		unsigned free_slot = cache_register_first_free_slot();

		if (free_slot == cache_register_invalid_slot) {
			return NULL;
		}
		error_descriptor * desc_ 
			= & error_descriptor_cache[free_slot];
				desc_->runtime_index = free_slot;
				desc_->line = line_;
				desc_->error_code = code_;
		// file_ length can be above error_descriptor_buffer_size
		// so this might clip the result
		strncpy(desc_->file, file_, error_descriptor_buffer_size );
		strncpy(desc_->error_message, msg_, error_descriptor_buffer_size );

		return desc_ ;
	}


static error_descriptor *  release_error_descriptor(error_descriptor ** locdesc_ )
{
	// make the empty descriptor, once
	static error_descriptor 
		empty_error_descriptor = { 0, 0, {0}, 0, {0} };

	assert( NULL != *locdesc_);

	cache_register_release_slot( (**locdesc_).runtime_index );
		(**locdesc_) = empty_error_descriptor;

	return (*locdesc_) = NULL;
}

static error_descriptor *  find_error_descriptor(const int runtime_index_)
{
	// make the empty descriptor, once
	static error_descriptor
		empty_error_descriptor = { 0, 0, {0}, 0, {0} };

	assert(runtime_index_ > -1);
	assert(runtime_index_ < cache_register_last_slot );


	if (cache_register[runtime_index_] == CACHE_REGISTER_ACTV_MARK)
	{
		return & error_descriptor_cache[runtime_index_];
	}
	return & empty_error_descriptor;
}

// the interface construction -------------------------------------------------
dbj_error dbj_error_provider 
   = { create_error_descriptor, release_error_descriptor, find_error_descriptor };

// EOF

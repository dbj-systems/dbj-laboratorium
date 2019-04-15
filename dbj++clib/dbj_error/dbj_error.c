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

#include "dbj_error.h"
#include "dbj_error_codes.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// the cache internals  -----------------------------------------------
extern error_descriptor error_descriptor_cache[];
extern const unsigned cache_register_invalid_slot;
extern unsigned cache_register_first_free_slot();
extern void cache_register_release_slot(unsigned registry_slot_index);
extern void *  cache_find_active(const int runtime_index_);

// the implementation -------------------------------------------------

static const char * unknown_error_location = "Unknown error location";
// return NULL if cache overflow
static struct error_descriptor *
	create_error_descriptor( 
		const int line_, const char * file_ ,
		const int code_, const char * msg_ 
	)
	{
	if (file_ == NULL) file_ = unknown_error_location;
	if (msg_ == NULL)  msg_  = dbj_error_message(code_);

	assert(msg_);

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
		return desc_;
	}

/*
DBJ_NOT_ERR is -1 so if accidentaly used in the cache it will 
produce compile time or runtime error for sure
*/
static error_descriptor * null_error_descriptor() {
	// make the empty descriptor, once
	static error_descriptor
		null_error_descriptor_ = { DBJ_NOT_ERR, DBJ_NOT_ERR, {0}, DBJ_NOT_ERR, {0} };
	return &null_error_descriptor_;
}

static bool is_null_error_descriptor( error_descriptor * ed_ ) {
	assert(ed_);
	return (
		(ed_ ) &&
		(ed_->error_code == DBJ_NOT_ERR) &&
		(ed_->error_message[0] == 0) &&
		(ed_->file[0] == 0) &&
		(ed_->line == DBJ_NOT_ERR) &&
		(ed_->runtime_index == DBJ_NOT_ERR)
		);
}

/*
valid error_descriptor is the one that is
-- not a null pointer
-- not empty
-- is active
*/
static bool is_valid_error_descriptor(error_descriptor * descriptor_) 
{
	if (0 == descriptor_) return false;
	// comparing the pointers ... ?
	if (is_null_error_descriptor( descriptor_)) return false;
	if (0 == cache_find_active(descriptor_->runtime_index)) return false;
	return true;
}

/*
make sure the error_descriptor argument
represents the one in use, not already released
or the one never made
*/
static void release_error_descriptor(error_descriptor ** locdesc_ )
{
	if (is_valid_error_descriptor(*locdesc_)) {
		cache_register_release_slot((**locdesc_).runtime_index);
	}
	if (*locdesc_)
		(**locdesc_) = * null_error_descriptor() ;
}

static error_descriptor *  find_error_descriptor(const int runtime_index_)
{
	void * cache_result = cache_find_active(runtime_index_);
	if (cache_result) return (error_descriptor *)cache_result;
	return null_error_descriptor() ;
}

// the interface construction -------------------------------------------------
dbj_error dbj_error_service 
   = { 
	create_error_descriptor, 
	release_error_descriptor, 
	find_error_descriptor, 
	is_valid_error_descriptor 
};

// EOF

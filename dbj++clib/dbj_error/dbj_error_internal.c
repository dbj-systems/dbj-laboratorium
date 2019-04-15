#include "../dbjclib.h"
#include "dbj_error.h"
#include "dbj_error_codes.h"

#define location_descriptor_cache_size error_descriptor_buffer_size

error_descriptor
error_descriptor_cache[location_descriptor_cache_size];

// we maintain separate array of "slots" to match the cache array 
// if register slot is false, the slot is free and if true the slot is used
#define CACHE_REGISTER_FREE_MARK false
#define CACHE_REGISTER_ACTV_MARK true
static bool cache_register[location_descriptor_cache_size];

const unsigned cache_register_invalid_slot = (unsigned)-1;

static unsigned cache_register_in_overflow_state = false;
static const unsigned cache_register_last_slot = error_descriptor_buffer_size - 1;
static const unsigned cache_register_first_slot = 0;


unsigned cache_register_first_free_slot() {

	if (cache_register_in_overflow_state) return cache_register_invalid_slot;

	// search for the first free slot
	for (unsigned cache_register_walker = cache_register_first_slot;
		cache_register_walker != cache_register_last_slot;
		cache_register_walker++)
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

void cache_register_release_slot(unsigned registry_slot_index)
{
	cache_register[registry_slot_index] = CACHE_REGISTER_FREE_MARK;
	// if here we can not have overflow situation
	cache_register_in_overflow_state = false;
};

void *  cache_find_active(const int runtime_index_)
{
	assert(runtime_index_ > -1);
	assert(runtime_index_ < cache_register_last_slot);

	if (cache_register[runtime_index_] == CACHE_REGISTER_ACTV_MARK)
	{
		return &error_descriptor_cache[runtime_index_];
	}
	return (void *)0 ;
}

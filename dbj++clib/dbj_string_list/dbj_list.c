
#include "dbj_string_list.h"
#include <string.h>
#include <assert.h>

static unsigned dbj_string_list_max_size = 0xFFFF;

static const char dbj_string_list_sentinel_char = '\033'; // ESC aka ((char)127);

typedef char dbj_string_list_value_type;

static const dbj_string_list_value_type * dbj_string_list_sentinel_		
	= (dbj_string_list_value_type *)(&dbj_string_list_sentinel_char);

dbj_string_list_type dbj_string_list_new()
{
	dbj_string_list_type empty_ = 0;
	empty_ = (dbj_string_list_type)DBJ_MALLOC(sizeof(empty_));
	assert(empty_);

	*empty_ = (char*)dbj_string_list_sentinel_;

	return empty_;
}

// return the pointer to the sentinel element
static dbj_string_list_type dbj_string_list_sentinel_ptr(dbj_string_list_type head_)
{
	dbj_string_list_type walker_ = head_;
	unsigned counter = 0;
	while (1 == 1) {
		if (*walker_ == dbj_string_list_sentinel_) break;
		walker_++;
		assert(counter++ < dbj_string_list_max_size);
	}
	return (walker_);
}

dbj_string_list_type dbj_string_list_append
(dbj_string_list_type head_, 
	const dbj_string_list_value_type * str_)
{
	assert(str_);
	dbj_string_list_type end_ = dbj_string_list_sentinel_ptr(head_);

	// check if we have the overflow
	size_t current_count_ = (end_ - head_);
	assert(current_count_ < dbj_string_list_max_size);

	// we expand +2
	// one for the new payload and one for the sentinel
	dbj_string_list_type tmp
		= (dbj_string_list_type)realloc(head_, (current_count_ + 2) * sizeof(tmp));

	assert(tmp);
	// point to resized block just after it is OK
	head_ = tmp;

	// attention: count and index are not the same
	head_[current_count_ + 0] = _strdup(str_);
	// removing the constness
	head_[current_count_ + 1] 
		= (dbj_string_list_value_type *)dbj_string_list_sentinel_;

	return head_;
}


void dbj_string_list_free(dbj_string_list_type head_)
{
	assert(head_);
	// not empty?
	if (dbj_string_list_sentinel_ != *head_) {
		// 
		dbj_string_list_type end_ = dbj_string_list_sentinel_ptr(head_);
		char * last_word_ = 0;
		// we skip over the sentinel
		// as the space for it was not allocated
		end_ -= 1;
		while (end_ != head_) {
			last_word_ = *end_;
			free(last_word_);
			last_word_ = *end_ = 0;
			end_ -= 1;
		};
	}
	if ( head_) {
		free(head_);
		head_ = 0  ;
	}
	// free(head_);
}

dbj_string_list_type dbj_string_list_reset(dbj_string_list_type head_)
{
	dbj_string_list_free(head_);
	return dbj_string_list_new();
}


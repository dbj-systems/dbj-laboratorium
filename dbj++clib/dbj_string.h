#pragma once
#include "dbjclib.h"
#ifdef __clang__
/*
http://clang.llvm.org/docs/UsersManual.html#controlling-diagnostics-in-system-headers
*/
#pragma clang system_header
#endif /* __clang__ */

static const size_t		DBJ_NPOS = (size_t)-1;
static const size_t		DBJ_MAX_STRING_LENGTH = 65536;
/*
Concept 1: string is two pointers, front and back
char * str = "ABC" ;
char * front = str ;
char * back  = str + strlen(str) ;

Effectively this is the concept of a 'range'
Sub-range is two pointers to the *same* buffer

Concept 2: minimize the use of the string.h

*/

typedef struct dbj_string {
	bool   full_free;
	char * front;
	char * back;
} dbj_string;

static dbj_string * dbj_string_null()
{
	dbj_string * pair_ = (dbj_string *)malloc(sizeof(dbj_string));
	DBJ_ASSERT(pair_);
	pair_->front = 0; pair_->back = 0; pair_->full_free = false; return pair_;
}

static bool dbj_valid_string(const dbj_string * str)
{
	if (!str) return false;
	return (
		str->front && str->back && ((str->back - str->front) > 0)
		&& (DBJ_MAX_STRING_LENGTH > (str->back - str->front))
		);
}


static void dbj_string_free(dbj_string * str)
{
	DBJ_ASSERT(str);
	if (str->full_free) free((void*)str->front);
	free(str);
}

static const size_t dbj_string_len(const dbj_string * str_)
{
	DBJ_ASSERT(str_);
	DBJ_ASSERT(DBJ_MAX_STRING_LENGTH > (size_t)(str_->back - str_->front));
	return (size_t)(str_->back - str_->front);
}

/*
make from const char *
*/
static dbj_string *
dbj_string_make(const char * string_)
{
	const size_t slen = strlen(string_);
	DBJ_ASSERT(DBJ_MAX_STRING_LENGTH > slen);

	dbj_string * pair_ = (dbj_string *)malloc(sizeof(dbj_string));
	DBJ_ASSERT(pair_);
	/* front not to be freed */
	pair_->full_free = false;
	pair_->front = (char *)string_;
	/* NOTE! if string_ is empty, back == front */
	pair_->back = (char *)string_ + slen;
	DBJ_ASSERT((size_t)(pair_->back - pair_->front) == slen);
	return pair_;
}
/*
front of the allocated dbj_string has to be freed
*/
static dbj_string * dbj_string_alloc(size_t count)
{
	DBJ_ASSERT(DBJ_MAX_STRING_LENGTH > count);
	char * payload = (char*)calloc(count + 1, 1);
	DBJ_ASSERT(payload);
	dbj_string * rez = dbj_string_make(payload);
	// since we made it with the empty string 
	// the back is pointing to the front 
	// so we have to re-adjust it!
	rez->back = rez->front + count;
	return rez;
}

static dbj_string * dbj_string_append(
	const dbj_string * left_,
	const dbj_string * right_
)
{
	DBJ_ASSERT(dbj_valid_string(left_));
	DBJ_ASSERT(dbj_valid_string(right_));

	dbj_string * rezult_ = dbj_string_alloc(dbj_string_len(left_) + dbj_string_len(right_));
	char * w_ = 0;
	char * r_ = rezult_->front;

	for (w_ = left_->front; w_ != left_->back; ++w_) {
		*r_ = *w_; ++r_;
	}

	for (w_ = right_->front; w_ != right_->back; ++w_) {
		*r_ = *w_; ++r_;
	}

	DBJ_ASSERT((rezult_->back - rezult_->front) > 0);

	return rezult_;
}

/*
compare the contents of two strings,
return true if equal
*/
static bool dbj_string_compare(
	const dbj_string * left_,
	const dbj_string * right_
)
{
	DBJ_ASSERT(dbj_valid_string(left_));
	DBJ_ASSERT(dbj_valid_string(right_));

	if (dbj_string_len(left_) != dbj_string_len(right_)) return false;

	char * l_ = left_->front;
	char * r_ = right_->front;

	for (l_ = left_->front; l_ != left_->back; ++l_) {
		if (*l_ != *r_) return false;
		++r_;
	}
	return true;
}

/*
take sub range as requested
free the string struct eventually but not the front pointer
*/
static dbj_string * dbj_string_from(const char * str, size_t from_, size_t to_)
{
	from_ -= 1;
	// to_ -= 1; we do not move the 'to' left since the concept is 
	// back pointer is one beyond the last 'to'
	if ((to_ - from_) > strlen(str)) {
		errno = EINVAL; return NULL;
	}
	dbj_string * retval = dbj_string_null();
	// full_free is false here
	retval->front = (char *)& str[from_];
	retval->back = (char *)& str[to_];
	return retval;
}
/*
is CONTENT of sub inside the CONTENT of str ?

return the sub-range as dbj_string with pointers to the same buffer
or NULL , with errno set to the the error
*/
static dbj_string *  dbj_to_subrange(dbj_string * str_, dbj_string * sub_)
{
	DBJ_ASSERT(str_ && sub_);
	DBJ_ASSERT(dbj_string_len(str_) > 0);
	DBJ_ASSERT(dbj_string_len(sub_) > 0);
	DBJ_ASSERT(dbj_string_len(sub_) < dbj_string_len(str_));

	dbj_string * sub_range_ = 0;

	/* outer loop is walk along the string */
	for (char * sp = str_->front; sp < str_->back; ++sp) {
		/* potenial substr start */
		if (*sp == *sub_->front)
		{
			char * sub_start_location = sp;
			bool sub_found_flag = false;
			/* walk along the potential substring */
			for (char * sw = sub_->front; sw != sub_->back; ++sw)
			{
				/* compare sub and str char's*/
				sub_found_flag = (*sw == *sp);
				/* and move the outer walker to */
				++sp;
				/* finish the loop */
				if (!sub_found_flag) break;
			}
			/* inner loop has finished    */
			/* and there where no misses? */
			if (sub_found_flag) {
				sub_range_ = dbj_string_null();
				sub_range_->front = sub_start_location;
				sub_range_->back = sp;
				return sub_range_;
				// marked for partial free-ing
			}
		}
	}
	/* none found */
	errno = EINVAL;	return 0;
}

/*
	is pointer p pointing inside the string range?
	return DBJ_NPOS if not found
*/
static const size_t dbj_p_is_in_range(const char * p_, dbj_string * str_) {

	/* walk along the source */
	char * char_p = (char *)p_;
	for (char * walker = str_->front; *walker != *str_->back; ++walker)
	{
		if (char_p == walker) return (size_t)(walker - str_->front);
	}
	return DBJ_NPOS;
}
/*
	is character c inside
	the string content
	return the location or DBJ_NPOS if not found
*/
static const size_t dbj_c_is_in_range(const char c_, dbj_string * str_)
{
	/* walk along the source */
	for (char * walker = str_->front; *walker != *str_->back; ++walker)
	{
		if (c_ == *walker) return (size_t)(walker - str_->front);
	}
	return DBJ_NPOS;
}

/*
return append left and right of a sub_range
*/
static dbj_string * dbj_remove_substring
(dbj_string * range, dbj_string * sub_range)
{
	// the big sanity check first
	if (!((range->front < sub_range->front) && (sub_range->back < range->back)))
	{
		errno = EINVAL; return 0;
	}

	dbj_string * left = dbj_string_null();
	dbj_string * right = dbj_string_null();

	left->front = range->front; left->back = sub_range->front;
	right->front = sub_range->back; right->back = range->back;

	dbj_string * rez = dbj_string_append(left, right);

	dbj_string_free(left);
	dbj_string_free(right);

	return rez;
}

static void dbj_string_test()
{
	// specimen starts from 1
	// thus sub(5,7) is '567'

	dbj_string * sub = dbj_string_from("456", 1, 3);

	DBJ_ASSERT(
		/* "456" == "456" */
		dbj_string_compare(dbj_string_from("12456", 3, 5), dbj_string_from("45612", 1, 3))
	);

	dbj_string * o2z = dbj_string_make("1234567890");
	// this yields DBJ_NPOS since "3" is different memory chunk 
	DBJ_ASSERT( DBJ_NPOS == dbj_p_is_in_range("3", o2z));
	// this yields 2
	DBJ_ASSERT(2 == dbj_p_is_in_range(o2z->front + 2, o2z));
	// this yields 2
	DBJ_ASSERT( 2 == dbj_c_is_in_range('3', o2z));
	/*
	if subrange is made, that means sub is
	found to be a substring  of the string
	by comparing the *contents*
	*/
	dbj_string * sub_range = dbj_to_subrange(o2z, sub);
	DBJ_ASSERT(sub_range);

	/*
	thus we can use it to remove the subrange
	*/
	dbj_string * rez = dbj_remove_substring(o2z, sub_range);

	dbj_string_free(sub_range);
	dbj_string_free(sub);
	dbj_string_free(rez);
}


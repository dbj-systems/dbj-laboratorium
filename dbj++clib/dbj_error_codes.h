#pragma once

typedef enum dbj_error_code 
{
	// note: we could go into negative numbers here
	// but that would produce coding trouble when
	// using witg standard ISO C error codes
	// hopefully ISO C + POSIX + whatever
	// error codes will not reach 1000
	DBJ_EC_INVALID_ARGUMENT = 1001,
	DBJ_EC_INDEX_TOO_SMALL,
	DBJ_EC_INDEX_TOO_LARGE,
	DBJ_EC_BAD_STD_ERR_CODE,
	DBJ_EC_BAD_ERR_CODE,
	DBJ_EC_DBJ_LAST_ERROR

} dbj_error_code ;

const char * dbj_error_message(unsigned int ec_);

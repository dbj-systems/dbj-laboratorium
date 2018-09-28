#include "dbjclib.h"

static const char * dbj_error_messages_[DBJ_EC_DBJ_LAST_ERROR] =
{
	"Invalid argument", /* EINVAL */
	"Index is too small",
	"Index is too large",
	"Bad ISO or POSIX errnum",
	"Bad DBJ error code", /* DBJ_EC_BAD_ERR_CODE */
	(char*)0
}; // eof dbj_error_messages

char last_error_msg[256] = {0};
char * dbj_str_error(char msg_[256], int err_code);

// in C dbj_error_code can be any int ...sigh ....
const char * dbj_error_message(unsigned int ec_)
{
	char * retval_ = 0;
	memset(last_error_msg, 0, 256);

	if (ec_ < DBJ_EC_INVALID_ARGUMENT) {

		// thus we will look for a ISO or POSIX message
		char * std_iso_message_ = dbj_str_error(last_error_msg, ec_);

		if (!std_iso_message_) {
			retval_ = (char *)dbj_error_messages_[DBJ_EC_BAD_STD_ERR_CODE];
		}
		else {
			retval_ = std_iso_message_;
		}
	}
	else if (ec_ >  DBJ_EC_DBJ_LAST_ERROR  )
		retval_ = (char *)dbj_error_messages_[DBJ_EC_BAD_ERR_CODE];
	else if (ec_ == DBJ_EC_DBJ_LAST_ERROR  ) 
		retval_ = (char *)dbj_error_messages_[DBJ_EC_BAD_ERR_CODE];
	else {
		retval_ = (char *)dbj_error_messages_[ec_];
	}
	return retval_;
}

// msg_ must be a buffer of len 256
char * dbj_str_error(char msg_[256], int err_code )
{
#ifdef _MSC_VER
	if (strerror_s(msg_, 256, err_code) != 0)
	{
		strncpy(msg_, "Unknown error", 256);
	}
#else
	if (strerror_r(err_code, msg_, 256) != 0)
	{
		strncpy(msg_, "Unknown error", 256);
	}
#endif
	msg_[256 - 1] = '\0';
	return &msg_[0];
}